/****************************************************************************
 * drivers/can/can.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <assert.h>
#include <poll.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/clock.h>
#include <nuttx/signal.h>
#include <nuttx/fs/fs.h>
#include <nuttx/can/can.h>
#include <nuttx/can/can_sender.h>
#include <nuttx/kmalloc.h>
#include <nuttx/irq.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

#ifdef CONFIG_CAN_TXREADY
#  if !defined(CONFIG_SCHED_WORKQUEUE)
#    error Work queue support required in this configuration
#    undef CONFIG_CAN_TXREADY
#    undef CONFIG_CAN_TXREADY_LOPRI
#    undef CONFIG_CAN_TXREADY_HIPRI
#  elif defined(CONFIG_CAN_TXREADY_LOPRI)
#    undef CONFIG_CAN_TXREADY_HIPRI
#    ifdef CONFIG_SCHED_LPWORK
#      define CANWORK LPWORK
#    else
#      error Low priority work queue support required in this configuration
#      undef CONFIG_CAN_TXREADY
#      undef CONFIG_CAN_TXREADY_LOPRI
#    endif
#  elif defined(CONFIG_CAN_TXREADY_HIPRI)
#    ifdef CONFIG_SCHED_HPWORK
#      define CANWORK HPWORK
#    else
#      error High priority work queue support required in this configuration
#      undef CONFIG_CAN_TXREADY
#      undef CONFIG_CAN_TXREADY_HIPRI
#    endif
#  else
#    error No work queue selection
#    undef CONFIG_CAN_TXREADY
#  endif
#endif

/* Timing Definitions *******************************************************/

#define HALF_SECOND_MSEC 500
#define HALF_SECOND_USEC 500000L

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

#ifdef CONFIG_CAN_TXREADY
static void           can_txready_work(FAR void *arg);
#endif

/* Character driver methods */

static int            can_open(FAR struct file *filep);
static int            can_close(FAR struct file *filep);
static ssize_t        can_read(FAR struct file *filep, FAR char *buffer,
                               size_t buflen);
static int            can_xmit(FAR struct can_dev_s *dev);
static ssize_t        can_write(FAR struct file *filep,
                                FAR const char *buffer, size_t buflen);
static inline ssize_t can_rtrread(FAR struct file *filep,
                                  FAR struct canioc_rtr_s *rtr);
static int            can_ioctl(FAR struct file *filep, int cmd,
                                unsigned long arg);
static int            can_poll(FAR struct file *filep,
                               FAR struct pollfd *fds,
                               bool setup);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct file_operations g_canops =
{
  can_open,  /* open */
  can_close, /* close */
  can_read,  /* read */
  can_write, /* write */
  NULL,      /* seek */
  can_ioctl, /* ioctl */
  NULL,      /* mmap */
  NULL,      /* truncate */
  can_poll   /* poll */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: can_txready_work
 *
 * Description:
 *   This function performs deferred processing from can_txready.  See the
 *   description of can_txready below for additional information.
 *
 ****************************************************************************/

#ifdef CONFIG_CAN_TXREADY
static void can_txready_work(FAR void *arg)
{
  FAR struct can_dev_s *dev = (FAR struct can_dev_s *)arg;
  irqstate_t flags;
  int ret;

  caninfo("xmit pending_count: %d sending_count: %d free_space: %d\n",
          PENDING_COUNT(&dev->cd_sender), SENDING_COUNT(&dev->cd_sender),
          FREE_COUNT(&dev->cd_sender));

  /* Verify that the sender is not empty.  The following operations must
   * be performed with interrupt disabled.
   */

  flags = enter_critical_section();
  if (!TX_EMPTY(&dev->cd_sender))
    {
      /* Send the next message in the sender. */

      ret = can_xmit(dev);

      /* If the message was successfully queued in the H/W sender, then
       * can_txdone() should have been called.  If the S/W sender were
       * full before then there should now be free space in the S/W sender.
       */

      if (ret >= 0)
        {
          /* Are there any threads waiting for space in the sender? */

          if (dev->cd_ntxwaiters > 0)
            {
              /* Yes.. Inform them that new xmit space is available */

              nxsem_post(&dev->cd_sender.tx_sem);
            }
        }
    }

  leave_critical_section(flags);
}
#endif

static FAR struct can_reader_s *init_can_reader(FAR struct file *filep)
{
  FAR struct can_reader_s *reader = kmm_zalloc(sizeof(struct can_reader_s));
  DEBUGASSERT(reader != NULL);

  nxsem_init(&reader->fifo.rx_sem, 0, 0);
  filep->f_priv = reader;

  return reader;
}

/****************************************************************************
 * Name: can_open
 *
 * Description:
 *   This function is called whenever the CAN device is opened.
 *
 ****************************************************************************/

static int can_open(FAR struct file *filep)
{
  FAR struct inode     *inode = filep->f_inode;
  FAR struct can_dev_s *dev   = inode->i_private;
  irqstate_t            flags;
  int                   ret;

  /* If the port is the middle of closing, wait until the close is finished */

  ret = nxmutex_lock(&dev->cd_closelock);
  if (ret < 0)
    {
      return ret;
    }

  /* If this is the first time that the driver has been opened
   * for this device, then perform hardware initialization.
   */

  caninfo("ocount: %u\n", dev->cd_crefs);

  if (dev->cd_crefs >= 255)
    {
      /* Limit to no more than 255 opens */

      ret = -EMFILE;
      goto errout;
    }
  else
    {
      flags = enter_critical_section();

      if (dev->cd_crefs == 0)
        {
          ret = dev_setup(dev);
          if (ret == OK)
            {
              /* Mark the sender empty */

              can_sender_init(&dev->cd_sender);

              /* Finally, Enable the CAN RX interrupt */

              dev_rxint(dev, true);
            }
        }

      if (ret == OK)
        {
          dev->cd_crefs++;

          /* Update the reader list only if driver was open for reading */

          if ((filep->f_oflags & O_RDOK) != 0)
            {
              list_add_head(&dev->cd_readers,
                            (FAR struct list_node *)init_can_reader(filep));
            }
        }

      leave_critical_section(flags);
    }

errout:
  nxmutex_unlock(&dev->cd_closelock);
  return ret;
}

/****************************************************************************
 * Name: can_close
 *
 * Description:
 *   This routine is called when the CAN device is closed.
 *   It waits for the last remaining data to be sent.
 *
 ****************************************************************************/

static int can_close(FAR struct file *filep)
{
  FAR struct inode     *inode = filep->f_inode;
  FAR struct can_dev_s *dev   = inode->i_private;
  irqstate_t            flags;
  FAR struct list_node *node;
  int                   ret;

#ifdef  CONFIG_DEBUG_CAN_INFO
  caninfo("ocount: %u\n", dev->cd_crefs);
#endif

  ret = nxmutex_lock(&dev->cd_closelock);
  if (ret < 0)
    {
      return ret;
    }

  flags = enter_critical_section(); /* Disable interrupts */

  list_for_every(&dev->cd_readers, node)
    {
      if (((FAR struct can_reader_s *)node) ==
          ((FAR struct can_reader_s *)filep->f_priv))
        {
          FAR struct can_reader_s *reader = (FAR struct can_reader_s *)node;
          FAR struct can_rxfifo_s *fifo   = &reader->fifo;

          /* Unlock the binary semaphore, waking up can_read if it
           * is blocked.
           */

          nxsem_post(&fifo->rx_sem);

          /* Notify specific poll/select waiter that they can read from the
           * cd_recv buffer
           */

          poll_notify(&reader->cd_fds, 1, POLLHUP);
          reader->cd_fds = NULL;
          list_delete(node);
          kmm_free(node);
          break;
        }
    }

  filep->f_priv = NULL;
  dev->cd_crefs--;

  /* De-initialize the driver if there are no more readers */

  if (dev->cd_crefs > 0)
    {
      goto errout;
    }

  /* Stop accepting input */

  dev_rxint(dev, false);

  /* Now we wait for the sender to clear */

  while (!TX_EMPTY(&dev->cd_sender))
    {
      nxsig_usleep(HALF_SECOND_USEC);
    }

  /* And wait for the hardware sender to drain */

  while (!dev_txempty(dev))
    {
      nxsig_usleep(HALF_SECOND_USEC);
    }

  /* Free the IRQ and disable the CAN device */

  dev_shutdown(dev); /* Disable the CAN */

errout:
  leave_critical_section(flags);
  nxmutex_unlock(&dev->cd_closelock);
  return ret;
}

/****************************************************************************
 * Name: can_read
 *
 * Description:
 *   Read standard CAN messages
 *
 ****************************************************************************/

static ssize_t can_read(FAR struct file *filep, FAR char *buffer,
                        size_t buflen)
{
  FAR struct can_reader_s *reader;
  FAR struct can_rxfifo_s *fifo;
  irqstate_t               flags;
  int                      ret = 0;

  caninfo("buflen: %zu\n", buflen);

  /* The caller must provide enough memory to catch the smallest possible
   * message.  This is not a system error condition, but we won't permit
   * it,  Hence we return 0.
   */

  if (buflen >= CAN_MSGLEN(0))
    {
      DEBUGASSERT(filep->f_priv != NULL);
      reader = (FAR struct can_reader_s *)filep->f_priv;
      fifo = &reader->fifo;

      /* Interrupts must be disabled while accessing the cd_recv FIFO */

      flags = enter_critical_section();

#ifdef CONFIG_CAN_ERRORS
      /* Check for internal errors */

      if (fifo->rx_error != 0)
        {
          FAR struct can_msg_s *msg;

          /* Detected an internal driver error.  Generate a
           * CAN_ERROR_MESSAGE
           */

          if (buflen < CAN_MSGLEN(CAN_ERROR_DLC))
            {
              goto return_with_irqdisabled;
            }

          msg                   = (FAR struct can_msg_s *)buffer;
          msg->cm_hdr.ch_id     = CAN_ERROR_INTERNAL;
          msg->cm_hdr.ch_dlc    = CAN_ERROR_DLC;
          msg->cm_hdr.ch_rtr    = 0;
          msg->cm_hdr.ch_error  = 1;
#ifdef CONFIG_CAN_EXTID
          msg->cm_hdr.ch_extid  = 0;
#endif
          msg->cm_hdr.ch_tcf    = 0;
          memset(&(msg->cm_data), 0, CAN_ERROR_DLC);
          msg->cm_data[5]       = fifo->rx_error;

          /* Reset the error flag */

          fifo->rx_error        = 0;

          ret = CAN_MSGLEN(CAN_ERROR_DLC);
          goto return_with_irqdisabled;
        }
#endif /* CONFIG_CAN_ERRORS */

      if ((filep->f_oflags & O_NONBLOCK) != 0)
        {
          ret = nxsem_trywait(&fifo->rx_sem);
        }
      else
        {
          ret = nxsem_wait(&fifo->rx_sem);
        }

      if (ret < 0)
        {
          goto return_with_irqdisabled;
        }

      if (fifo->rx_head == fifo->rx_tail)
        {
          canerr("RX FIFO sem posted but FIFO is empty.\n");
          goto return_with_irqdisabled;
        }

      /* The cd_recv FIFO is not empty.  Copy all buffered data that will fit
       * in the user buffer.
       */

      do
        {
          /* Will the next message in the FIFO fit into the user buffer? */

          FAR struct can_msg_s *msg = &fifo->rx_buffer[fifo->rx_head];
          int nbytes = can_dlc2bytes(msg->cm_hdr.ch_dlc);
          int msglen = CAN_MSGLEN(nbytes);

          if (ret + msglen > buflen)
            {
              break;
            }

          /* Copy the message to the user buffer */

          memcpy(&buffer[ret], msg, msglen);
          ret += msglen;

          /* Increment the head of the circular message buffer */

          if (++fifo->rx_head >= CONFIG_CAN_RXFIFOSIZE)
            {
              fifo->rx_head = 0;
            }
        }
      while (fifo->rx_head != fifo->rx_tail);

      if (fifo->rx_head != fifo->rx_tail)
        {
          /* The user's buffer was too small, so some messages remain in the
           * FIFO. Post the semaphore so future calls to poll() or read()
           * don't block.
           */

          nxsem_post(&fifo->rx_sem);
        }

return_with_irqdisabled:
      leave_critical_section(flags);
    }

  return ret;
}

/****************************************************************************
 * Name: can_xmit
 *
 * Description:
 *   Send the message at the head of the sender
 *
 * Assumptions:
 *   Called with interrupts disabled
 *
 ****************************************************************************/

static int can_xmit(FAR struct can_dev_s *dev)
{
  FAR struct can_msg_s *msg;
  int ret = -EBUSY;

  caninfo("xmit pending_count: %d sending_count: %d free_space: %d\n",
          PENDING_COUNT(&dev->cd_sender), SENDING_COUNT(&dev->cd_sender),
          FREE_COUNT(&dev->cd_sender));

  /* If there is nothing to send, then just disable interrupts and return */

  if (TX_EMPTY(&dev->cd_sender))
    {
      DEBUGASSERT(SENDING_COUNT(&dev->cd_sender) == 0);

#ifndef CONFIG_CAN_TXREADY
      /* We can disable CAN TX interrupts -- unless there is a H/W sender. In
       * that case, TX interrupts must stay enabled until the H/W sender is
       * fully emptied.
       */

      dev_txint(dev, false);
#endif
      return -EIO;
    }

  /* Check if we have already queued all of the data in the sender.
   *
   * tx_tail:  Incremented in can_write each time a message is queued in the
   *           sender
   * tx_head:  Incremented in can_txdone each time a message completes
   * tx_queue: Incremented each time that a message is sent to the hardware.
   *
   * Logically (ignoring buffer wrap-around): tx_head <= tx_queue <= tx_tail
   * tx_head == tx_queue == tx_tail means that the sender is empty
   * tx_head < tx_queue == tx_tail means that all data has been queued, but
   * we are still waiting for transmissions to complete.
   */

  while (TX_PENDING(&dev->cd_sender) && dev_txready(dev))
    {
      /* No.. The sender should not be empty in this case */

      DEBUGASSERT(!TX_EMPTY(&dev->cd_sender));

      msg = can_get_msg(&dev->cd_sender);

      if (msg == NULL)
        {
          break;
        }

      /* Send the next message at the sender */

      ret = dev_send(dev, msg);
      if (ret < 0)
        {
          canerr("dev_send failed: %d\n", ret);
          can_revert_msg(&dev->cd_sender, msg);
          break;
        }
    }

  /* Make sure that TX interrupts are enabled */

  dev_txint(dev, true);
  return ret;
}

/****************************************************************************
 * Name: can_write
 ****************************************************************************/

static ssize_t can_write(FAR struct file *filep, FAR const char *buffer,
                         size_t buflen)
{
  FAR struct inode         *inode   = filep->f_inode;
  FAR struct can_dev_s     *dev     = inode->i_private;
  FAR struct can_txcache_s *sender  = &dev->cd_sender;
  FAR struct can_msg_s     *msg;
  bool                      inactive;
  ssize_t                   nsent   = 0;
  irqstate_t                flags;
  int                       nbytes;
  int                       msglen;
  int                       ret     = 0;

  caninfo("buflen: %zu\n", buflen);

  /* Interrupts must disabled throughout the following */

  flags = enter_critical_section();

  /* Check if the H/W TX is inactive when we started. In certain race
   * conditions, there may be a pending interrupt to kick things back off,
   * but we will be sure here that there is not.  That the hardware is IDLE
   * and will need to be kick-started.
   */

  inactive = dev_txempty(dev);

  /* Add the messages to the sender.  Ignore any trailing messages that are
   * shorter than the minimum.
   */

  while (buflen - nsent >= CAN_MSGLEN(0))
    {
      /* If the sender becomes full, then wait for space to become
       * available.
       */

      while (TX_FULL(sender))
        {
          /* The transmit sender is full  -- non-blocking mode selected? */

          if ((filep->f_oflags & O_NONBLOCK) != 0)
            {
              if (nsent == 0)
                {
                  ret = -EAGAIN;
                }
              else
                {
                  ret = nsent;
                }

              goto return_with_irqdisabled;
            }

          /* If the TX hardware was inactive when we started, then we will
           * have start the XMIT sequence generate the TX done interrupts
           * needed to clear the sender.
           */

          if (inactive)
            {
              can_xmit(dev);
            }

          /* Wait for a message to be sent */

          DEBUGASSERT(dev->cd_ntxwaiters < 255);
          dev->cd_ntxwaiters++;
          ret = nxsem_wait(&sender->tx_sem);
          dev->cd_ntxwaiters--;
          if (ret < 0)
            {
              goto return_with_irqdisabled;
            }

          /* Re-check the H/W sender state */

          inactive = dev_txempty(dev);
        }

      /* We get here if there is space in sender.  Add the new
       * CAN message at sutibal.
       */

      msg    = (FAR struct can_msg_s *)&buffer[nsent];
      nbytes = can_dlc2bytes(msg->cm_hdr.ch_dlc);
      msglen = CAN_MSGLEN(nbytes);

      can_add_sendnode(sender, msg, msglen);

      /* Increment the number of bytes that were sent */

      nsent += msglen;
    }

  /* We get here after all messages have been added to the sender.  Check if
   * we need to kick off the XMIT sequence.
   */

  if (inactive)
    {
      can_xmit(dev);
    }

  /* Return the number of bytes that were sent */

  ret = nsent;

return_with_irqdisabled:
  leave_critical_section(flags);
  return ret;
}

/****************************************************************************
 * Name: can_rtrread
 *
 * Description:
 *   Read RTR messages.  The RTR message is a special message -- it is an
 *   outgoing message that says "Please re-transmit the message with the
 *   same identifier as this message.  So the RTR read is really a
 *   send-wait-receive operation.
 *
 ****************************************************************************/

static inline ssize_t can_rtrread(FAR struct file *filep,
                                  FAR struct canioc_rtr_s *request)
{
  FAR struct can_dev_s     *dev = filep->f_inode->i_private;
  FAR struct can_rtrwait_s *wait = NULL;
  int                       i;
  int                       sval;
  int                       ret = -ENOMEM;

  /* Find an available slot in the pending RTR list */

  for (i = 0; i < CONFIG_CAN_NPENDINGRTR; i++)
    {
      FAR struct can_rtrwait_s *tmp = &dev->cd_rtr[i];

      ret = nxsem_get_value(&tmp->cr_sem, &sval);
      if (ret < 0)
        {
          continue;
        }

      if (sval == 0)
        {
          /* No one is waiting on RTR transaction; take it. */

          tmp->cr_msg     = request->ci_msg;
          dev->cd_npendrtr++;
          wait            = tmp;
          break;
        }
    }

  if (wait)
    {
      /* Send the remote transmission request with the "old method" unless
       * the lower-half driver indicates otherwise.
       */

      if (dev->cd_ops->co_remoterequest != NULL)
        {
          if (request->ci_msg->cm_hdr.ch_id < CAN_MAX_STDMSGID
#ifdef CONFIG_CAN_EXTID
              && !request->ci_msg->cm_hdr.ch_extid
#endif
            )
            {
              ret = dev_remoterequest(dev,
                                (uint16_t)(request->ci_msg->cm_hdr.ch_id));
            }
          else
            {
              ret = -EINVAL;
            }
        }
      else
        {
#ifdef CONFIG_CAN_USE_RTR
          /* Temporarily set the RTR bit, then send the remote transmission
           * request message with the lower-half driver's regular function.
           */

          request->ci_msg->cm_hdr.ch_rtr = 1;
          ret = can_write(filep,
                          (FAR const char *)request->ci_msg,
                          CAN_MSGLEN(request->ci_msg->cm_hdr.ch_dlc));
          request->ci_msg->cm_hdr.ch_rtr = 0;
#else
          canerr("Error: Driver needs CONFIG_CAN_USE_RTR.\n");
          ret = -ENOSYS;
#endif
        }

      if (ret >= 0)
        {
          /* Then wait for the response */

          ret = nxsem_tickwait(&wait->cr_sem,
                               SEC2TICK(request->ci_timeout.tv_sec) +
                               NSEC2TICK(request->ci_timeout.tv_nsec));
        }
    }

  return ret;
}

/****************************************************************************
 * Name: can_ioctl
 ****************************************************************************/

static int can_ioctl(FAR struct file *filep, int cmd, unsigned long arg)
{
  FAR struct inode        *inode  = filep->f_inode;
  FAR struct can_dev_s    *dev    = inode->i_private;
  FAR struct can_reader_s *reader = filep->f_priv;
  int                      ret    = OK;
  irqstate_t               flags;

  caninfo("cmd: %d arg: %ld\n", cmd, arg);

  /* Disable interrupts through this operation */

  flags = enter_critical_section();

  /* Handle built-in ioctl commands */

  switch (cmd)
    {
      /* CANIOC_RTR: Send the remote transmission request and wait for the
       * response.  Argument is a reference to struct canioc_rtr_s
       * (casting to uintptr_t first eliminates complaints on some
       * architectures where the sizeof long is different from the size of
       * a pointer).
       */

      case CANIOC_RTR:
        {
          ret = can_rtrread(filep,
                            (FAR struct canioc_rtr_s *)((uintptr_t)arg));
        }
        break;

      /* CANIOC_IFLUSH: Flush data received but not read. No argument.      */

      case CANIOC_IFLUSH:
        {
          reader->fifo.rx_head = 0;
          reader->fifo.rx_tail = 0;

          /* invoke lower half ioctl */

          ret = dev_ioctl(dev, cmd, arg);
        }
        break;

      /* CANIOC_OFLUSH: Flush data written but not transmitted. No argument */

      case CANIOC_OFLUSH:
        {
          can_sender_init(&dev->cd_sender);

          /* invoke lower half ioctl */

          ret = dev_ioctl(dev, cmd, arg);
        }
        break;

      /* CANIOC_IOFLUSH: Flush data received but not read and data written
       *                 but not yet transmitted
       */

      case CANIOC_IOFLUSH:
        {
          can_sender_init(&dev->cd_sender);

          reader->fifo.rx_head = 0;
          reader->fifo.rx_tail = 0;

          /* invoke lower half ioctl */

          ret = dev_ioctl(dev, cmd, arg);
        }
        break;

      /* FIONWRITE: Return the number of CAN messages in the send queue */

      case FIONWRITE:
        {
          *(FAR int *)arg = PENDING_COUNT(&dev->cd_sender) -
                              SENDING_COUNT(&dev->cd_sender);
        }
        break;

      /* FIONREAD: Return the number of CAN messages in the receive FIFO */

      case FIONREAD:
        {
          *(FAR uint8_t *)arg =
#ifdef CONFIG_CAN_ERRORS
                            (reader->fifo.rx_error != 0) +
#endif
                            reader->fifo.rx_tail - reader->fifo.rx_head;
        }
        break;

      /* Set specific can transceiver state */

      case CANIOC_SET_TRANSVSTATE:
        {
          /* if we don't use dev->cd_transv->cts_ops, please initlize
           * this pointer to NULL in lower board code when Board reset.
           */

          if (dev->cd_transv && dev->cd_transv->ct_ops
              && dev->cd_transv->ct_ops->ct_setstate)
            {
              FAR const struct can_transv_ops_s *ct_ops =
                                                dev->cd_transv->ct_ops;
              ret = ct_ops->ct_setstate(dev->cd_transv, arg);
            }
          else
            {
              canerr("dev->cd_transv->cts_ops is NULL!");
              ret = -ENOTTY;
            }
        }
        break;

      /* Get specific can transceiver state */

      case CANIOC_GET_TRANSVSTATE:
        {
          /* if we don't use dev->cd_transv->cts_ops, please initlize
           * this pointer to NULL in lower board code when Board reset.
           */

          if (dev->cd_transv && dev->cd_transv->ct_ops
              && dev->cd_transv->ct_ops->ct_getstate)
            {
              int *state = (FAR int *)arg;
              FAR const struct can_transv_ops_s *ct_ops =
                                                dev->cd_transv->ct_ops;
              ret = ct_ops->ct_getstate(dev->cd_transv, state);
            }
          else
            {
              canerr("dev->cd_transv->cts_ops is NULL!");
              ret = -ENOTTY;
            }
        }
        break;

      /* Not a "built-in" ioctl command.. perhaps it is unique to this
       * lower-half, device driver.
       */

      default:
        {
          ret = dev_ioctl(dev, cmd, arg);
        }
        break;
    }

  leave_critical_section(flags);
  return ret;
}

/****************************************************************************
 * Name: can_poll
 ****************************************************************************/

static int can_poll(FAR struct file *filep, FAR struct pollfd *fds,
                    bool setup)
{
  FAR struct inode        *inode    = filep->f_inode;
  FAR struct can_dev_s    *dev      = inode->i_private;
  FAR struct can_reader_s *reader   = NULL;
  pollevent_t              eventset = 0;
  int                      ret      = OK;
  irqstate_t               flags;

  /* Some sanity checking */

#ifdef CONFIG_DEBUG_FEATURES
  if (dev == NULL || fds == NULL)
    {
      return -ENODEV;
    }
#endif

  /* Ensure exclusive access to sender indices - don't want can_receive or
   * can_read changing them in the middle of the comparison
   */

  flags = enter_critical_section();

  DEBUGASSERT(filep->f_priv != NULL);
  reader = (FAR struct can_reader_s *)filep->f_priv;

  /* Get exclusive access to the poll structures */

  ret = nxmutex_lock(&dev->cd_polllock);
  if (ret < 0)
    {
      /* A signal received while waiting for access to the poll data
       * will abort the operation
       */

      goto return_with_irqdisabled;
    }

  /* Are we setting up the poll?  Or tearing it down? */

  if (setup)
    {
      /* This is a request to set up the poll.  Find an available
       * slot for the poll structure reference.
       */

      if (reader->cd_fds != NULL)
        {
          fds->priv = NULL;
          ret       = -EBUSY;
          goto errout;
        }

      /* Have found an available slot,
       * bind the poll structure and this slot
       */

      reader->cd_fds = fds;
      fds->priv      = &reader->cd_fds;

      /* Should we immediately notify on any of the requested events?
       * First, check if the sender is full.
       */

      if (!TX_FULL(&dev->cd_sender))
        {
          eventset |= POLLOUT;
        }

      /* Check whether there are messages in the RX FIFO. */

      if (reader->fifo.rx_head != reader->fifo.rx_tail
#ifdef CONFIG_CAN_ERRORS
          || reader->fifo.rx_error != 0
#endif
         )
        {
          /* No need to wait, just notify the application immediately */

          eventset |= POLLIN;
        }

      poll_notify(&fds, 1, eventset);
    }
  else if (fds->priv != NULL)
    {
      /* This is a request to tear down the poll */

      FAR struct pollfd **slot = (FAR struct pollfd **)fds->priv;

#ifdef CONFIG_DEBUG_FEATURES
      if (slot == NULL)
        {
          ret = -EIO;
          goto errout;
        }
#endif

      /* Remove all memory of the poll setup */

      *slot     = NULL;
      fds->priv = NULL;
    }

errout:
  nxmutex_unlock(&dev->cd_polllock);
return_with_irqdisabled:
  leave_critical_section(flags);
  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: can_register
 *
 * Description:
 *   Register a CAN driver.
 *
 ****************************************************************************/

int can_register(FAR const char *path, FAR struct can_dev_s *dev)
{
  int i;

  /* Initialize the CAN device structure */

  dev->cd_crefs      = 0;
  dev->cd_npendrtr   = 0;
  dev->cd_ntxwaiters = 0;
  list_initialize(&dev->cd_readers);

  /* Initialize semaphores */

  nxsem_init(&dev->cd_sender.tx_sem, 0, 0);
  nxmutex_init(&dev->cd_closelock);
  nxmutex_init(&dev->cd_polllock);

  for (i = 0; i < CONFIG_CAN_NPENDINGRTR; i++)
    {
      /* Initialize wait semaphores.  These semaphores are used for signaling
       * and should not have priority inheritance enabled.
       */

      nxsem_init(&dev->cd_rtr[i].cr_sem, 0, 0);
    }

  /* Initialize/reset the CAN hardware */

  dev_reset(dev);

  /* Register the CAN device */

  caninfo("Registering %s\n", path);
  return register_driver(path, &g_canops, 0666, dev);
}

/****************************************************************************
 * Name: can_receive
 *
 * Description:
 *   Called from the CAN interrupt handler when new read data is available
 *
 * Input Parameters:
 *   dev  - CAN driver state structure
 *   hdr  - CAN message header
 *   data - CAN message data (if DLC > 0)
 *
 * Returned Value:
 *   OK on success; a negated errno on failure.
 *
 * Assumptions:
 *   CAN interrupts are disabled.
 *
 ****************************************************************************/

int can_receive(FAR struct can_dev_s *dev, FAR struct can_hdr_s *hdr,
                FAR uint8_t *data)
{
  FAR struct can_rxfifo_s *fifo;
  FAR struct list_node    *node;
  irqstate_t               flags;
  int                      nexttail;
  int                      ret = -ENOMEM;
  int                      i;
  int                      sval;

  caninfo("ID: %" PRId32 " DLC: %d\n", (uint32_t)hdr->ch_id, hdr->ch_dlc);

  flags = enter_critical_section();

  /* Check if adding this new message would over-run the drivers ability to
   * enqueue read data.
   */

  /* First, check if this response matches any RTR response that we may be
   * waiting for.
   */

  if (dev->cd_npendrtr > 0)
    {
      /* There are pending RTR requests -- search the lists of requests
       * and see any any matches this new message.
       */

      for (i = 0; i < CONFIG_CAN_NPENDINGRTR; i++)
        {
          FAR struct can_rtrwait_s *wait = &dev->cd_rtr[i];
          FAR struct can_msg_s     *waitmsg = wait->cr_msg;

          /* Check if the entry is in use and whether the ID matches */

          if (nxsem_get_value(&wait->cr_sem, &sval) < 0)
            {
              continue;
            }
          else if (sval < 0
#ifdef CONFIG_CAN_ERRORS
                   && hdr->ch_error == false
#endif
#ifdef CONFIG_CAN_EXTID
                   && waitmsg->cm_hdr.ch_extid == hdr->ch_extid
#endif
                   && waitmsg->cm_hdr.ch_id == hdr->ch_id)
            {
              int nbytes;

              /* We have the response... copy the data to the user's buffer */

              memcpy(&waitmsg->cm_hdr, hdr, sizeof(struct can_hdr_s));

              nbytes = can_dlc2bytes(hdr->ch_dlc);
              if (nbytes)
                {
                  memcpy(waitmsg->cm_data, data, nbytes);
                }

              dev->cd_npendrtr--;

              /* Restart the waiting thread and mark the entry unused */

              nxsem_post(&wait->cr_sem);
            }
        }
    }

  list_for_every(&dev->cd_readers, node)
    {
      FAR struct can_reader_s *reader = (FAR struct can_reader_s *)node;
      fifo = &reader->fifo;

      nexttail = fifo->rx_tail + 1;
      if (nexttail >= CONFIG_CAN_RXFIFOSIZE)
        {
          nexttail = 0;
        }

      /* Refuse the new data if the FIFO is full */

      if (nexttail != fifo->rx_head)
        {
          int nbytes;

          /* Add the new, decoded CAN message at the tail of the FIFO.
           *
           * REVISIT:  In the CAN FD format, the coding of the DLC differs
           * from the standard CAN format. The DLC codes 0 to 8 have the
           * same coding as in standard CAN, the codes 9 to 15, which in
           * standard CAN all code a data field of 8 bytes, are encoded:
           *
           *   9->12, 10->16, 11->20, 12->24, 13->32, 14->48, 15->64
           */

          memcpy(&fifo->rx_buffer[fifo->rx_tail].cm_hdr, hdr,
                 sizeof(struct can_hdr_s));

          nbytes = can_dlc2bytes(hdr->ch_dlc);
          if (nbytes)
            {
              memcpy(fifo->rx_buffer[fifo->rx_tail].cm_data, data, nbytes);
            }

          /* Increment the tail of the circular buffer */

          fifo->rx_tail = nexttail;

          if (nxsem_get_value(&fifo->rx_sem, &sval) < 0)
            {
#ifdef CONFIG_CAN_ERRORS
              /* Report unspecified error */

              fifo->rx_error |= CAN_ERROR5_UNSPEC;
#endif
              continue;
            }

          /* Unlock the binary semaphore, waking up can_read if it is
           * blocked. If can_read were not blocked, we would not be
           * executing this because interrupts would be disabled.
           */

          if (sval <= 0)
            {
              nxsem_post(&fifo->rx_sem);
            }

          /* Notify specific poll/select waiter that they can read from the
           * cd_recv buffer
           */

          poll_notify(&reader->cd_fds, 1, POLLIN);
          ret = OK;
        }
#ifdef CONFIG_CAN_ERRORS
      else
        {
          /* Report rx overflow error */

          fifo->rx_error |= CAN_ERROR5_RXOVERFLOW;
        }
#endif
    }

  leave_critical_section(flags);
  return ret;
}

/****************************************************************************
 * Name: can_txdone
 *
 * Description:
 *   Called when the hardware has processed the outgoing TX message.  This
 *   normally means that the CAN messages was sent out on the wire.  But
 *   if the CAN hardware supports a H/W TX sender, then this call may mean
 *   only that the CAN message has been added to the H/W sender.  In either
 *   case, the upper-half CAN driver can remove the outgoing message from
 *   the S/W sender and discard it.
 *
 *   This function may be called in different contexts, depending upon the
 *   nature of the underlying CAN hardware.
 *
 *   1. No H/W sender (CONFIG_CAN_TXREADY not defined)
 *
 *      This function is only called from the CAN interrupt handler at the
 *      completion of a send operation.
 *
 *        can_write() -> can_xmit() -> dev_send()
 *        CAN interrupt -> can_txdone()
 *
 *      If the CAN hardware is busy, then the call to dev_send() will
 *      fail, the S/W TX sender will accumulate outgoing messages, and the
 *      thread calling can_write() may eventually block waiting for space in
 *      the S/W sender.
 *
 *      When the CAN hardware completes the transfer and processes the
 *      CAN interrupt, the call to can_txdone() will make space in the S/W
 *      sender and will awaken the waiting can_write() thread.
 *
 *   2a. H/W sender (CONFIG_CAN_TXREADY=y) and S/W sender not full
 *
 *      This function will be called back from dev_send() immediately when a
 *      new CAN message is added to H/W sender:
 *
 *        can_write() -> can_xmit() -> dev_send() -> can_txdone()
 *
 *      When the H/W sender becomes full, dev_send() will fail and
 *      can_txdone() will not be called.  In this case the S/W sender will
 *      accumulate outgoing messages, and the thread calling can_write() may
 *      eventually block waiting for space in the S/W sender.
 *
 *   2b. H/W sender (CONFIG_CAN_TXREADY=y) and S/W sender full
 *
 *      In this case, the thread calling can_write() is blocked waiting for
 *      space in the S/W sender.  can_txdone() will be called, indirectly,
 *      from can_txready_work() running on the thread of the work queue.
 *
 *        CAN interrupt -> can_txready() -> Schedule can_txready_work()
 *        can_txready_work() -> can_xmit() -> dev_send() -> can_txdone()
 *
 *      The call dev_send() should not fail in this case and the subsequent
 *      call to can_txdone() will make space in the S/W sender and will
 *      awaken the waiting thread.
 *
 * Input Parameters:
 *   dev  - The specific CAN device
 *
 * Returned Value:
 *   OK on success; a negated errno on failure.
 *
 * Assumptions:
 *   Interrupts are disabled.  This is required by can_xmit() which is called
 *   by this function.  Interrupts are explicitly disabled when called
 *   through can_write().  Interrupts are expected be disabled when called
 *   from the CAN interrupt handler.
 *
 ****************************************************************************/

int can_txdone(FAR struct can_dev_s *dev)
{
  FAR struct list_node *node;
  int ret = -ENOENT;
  irqstate_t flags;

  caninfo("xmit pending_count: %d sending_count: %d free_space: %d\n",
          PENDING_COUNT(&dev->cd_sender), SENDING_COUNT(&dev->cd_sender),
          FREE_COUNT(&dev->cd_sender));

  flags = enter_critical_section();

  /* Verify that the sender is not empty */

  if (!TX_EMPTY(&dev->cd_sender))
    {
      /* The tx_queue index is incremented each time can_xmit() queues
       * the transmission.  When can_txdone() is called, the tx_queue
       * index should always have been advanced beyond the current tx_head
       * index.
       */

      DEBUGASSERT(SENDING_COUNT(&dev->cd_sender) != 0);

      /* Remove the message at the head of the sender */

      can_send_done(&dev->cd_sender);

      /* Send the next message in the sender */

      can_xmit(dev);

      /* Notify all poll/select waiters that they can write to the sender
       * buffer
       */

      list_for_every(&dev->cd_readers, node)
        {
          FAR struct can_reader_s *reader = (FAR struct can_reader_s *)node;
          poll_notify(&reader->cd_fds, 1, POLLOUT);
        }

      /* Are there any threads waiting for space in the sender? */

      if (dev->cd_ntxwaiters > 0)
        {
          /* Yes.. Inform them that new xmit space is available */

          ret = nxsem_post(&dev->cd_sender.tx_sem);
        }
      else
        {
          ret = OK;
        }
    }

  leave_critical_section(flags);
  return ret;
}

/****************************************************************************
 * Name: can_txready
 *
 * Description:
 *   Called from the CAN interrupt handler at the completion of a send
 *   operation.  This interface is needed only for CAN hardware that
 *   supports queueing of outgoing messages in a H/W sender.
 *
 *   The CAN upper half driver also supports a queue of output messages in a
 *   S/W sender.  Messages are added to that queue when when can_write() is
 *   called and removed from the queue in can_txdone() when each TX message
 *   is complete.
 *
 *   After each message is added to the S/W sender, the CAN upper half driver
 *   will attempt to send the message by calling into the lower half driver.
 *   That send will not be performed if the lower half driver is busy, i.e.,
 *   if dev_txready() returns false.  In that case, the number of messages in
 *   the S/W sender can grow.  If the S/W sender becomes full, then
 *   can_write() will wait for space in the S/W sender.
 *
 *   If the CAN hardware does not support a H/W sender then busy means that
 *   the hardware is actively sending the message and is guaranteed to
 *   become non-busy (i.e, dev_txready()) when the send transfer completes
 *   and can_txdone() is called.  So the call to can_txdone() means that the
 *   transfer has completed and also that the hardware is ready to accept
 *   another transfer.
 *
 *   If the CAN hardware supports a H/W sender, can_txdone() is not called
 *   when the transfer is complete, but rather when the transfer is queued in
 *   the H/W sender.  When the H/W sender becomes full, then dev_txready()
 *   will report false and the number of queued messages in the S/W sender
 *   will grow.
 *
 *   There is no mechanism in this case to inform the upper half driver when
 *   the hardware is again available, when there is again space in the H/W
 *   sender.  can_txdone() will not be called again.  If the S/W sender
 *   becomes full, then the upper half driver will wait for space to become
 *   available, but there is no event to awaken it and the driver will hang.
 *
 *   Enabling this feature adds support for the can_txready() interface.
 *   This function is called from the lower half driver's CAN interrupt
 *   handler each time a TX transfer completes.  This is a sure indication
 *   that the H/W sender is no longer full.  can_txready() will then awaken
 *   the can_write() logic and the hang condition is avoided.
 *
 * Input Parameters:
 *   dev  - The specific CAN device
 *
 * Returned Value:
 *   OK on success; a negated errno on failure.
 *
 * Assumptions:
 *   Interrupts are disabled.  This function may execute in the context of
 *   and interrupt handler.
 *
 ****************************************************************************/

#ifdef CONFIG_CAN_TXREADY
int can_txready(FAR struct can_dev_s *dev)
{
  int ret = -ENOENT;
  irqstate_t flags;

  caninfo("xmit pending_count: %d sending_count: %d free_space: %d"
          " waiters: %d\n",
          PENDING_COUNT(&dev->cd_sender), SENDING_COUNT(&dev->cd_sender),
          FREE_COUNT(&dev->cd_sender), dev->cd_ntxwaiters);

  flags = enter_critical_section();

  /* Verify that the sender is not empty.  This is safe because interrupts
   * are always disabled when calling into can_xmit(); this cannot collide
   * with ongoing activity from can_write().
   */

  if (!TX_EMPTY(&dev->cd_sender))
    {
      /* Is work already scheduled? */

      if (work_available(&dev->cd_work))
        {
          /* Yes... schedule to perform can_txready() work on the worker
           * thread.  Although data structures are protected by disabling
           * interrupts, the can_xmit() operations may involve semaphore
           * operations and, hence, should not be done at the interrupt
           * level.
           */

          ret = work_queue(CANWORK, &dev->cd_work, can_txready_work, dev, 0);
        }
      else
        {
          ret = -EBUSY;
        }
    }
  else
    {
      /* There should not be any threads waiting for space in the S/W sender
       * is it is empty.  However, an assertion would fire in certain
       * race conditions, i.e, when all waiters have been awakened but
       * have not yet had a chance to decrement cd_ntxwaiters.
       */

#if 0 /* REVISIT */
      /* When the H/W sender has been emptied, we can disable further TX
       * interrupts.
       *
       * REVISIT:  The fact that the S/W sender is empty does not mean that
       * the H/W sender is also empty.  If we really want this to work this
       * way, then we would probably need and additional parameter to tell
       * us if the H/W sender is empty.
       */

      dev_txint(dev, false);
#endif
    }

  leave_critical_section(flags);
  return ret;
}
#endif /* CONFIG_CAN_TXREADY */
