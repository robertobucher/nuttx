/****************************************************************************
 * fs/vfs/fs_poll.c
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

#include <poll.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/clock.h>
#include <nuttx/semaphore.h>
#include <nuttx/cancelpt.h>
#include <nuttx/fs/fs.h>
#include <nuttx/net/net.h>
#include <nuttx/tls.h>

#include <arch/irq.h>

#include "inode/inode.h"
#include "fs_heap.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct pollfd_s
{
  FAR struct pollfd *fds;
  nfds_t nfds;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: poll_teardown
 *
 * Description:
 *   Teardown the poll operation for each descriptor in the list and return
 *   the count of non-zero poll events.
 *
 ****************************************************************************/

static inline void poll_teardown(FAR struct pollfd *fds, nfds_t nfds,
                                 FAR int *count)
{
  unsigned int i;

  /* Process each descriptor in the list */

  *count = 0;
  for (i = 0; i < nfds; i++)
    {
      if (fds[i].fd >= 0)
        {
          FAR struct file *filep;
          int ret;

          ret = file_get(fds[i].fd, &filep);
          if (ret >= 0)
            {
              ret = file_poll(filep, &fds[i], false);

              /* Calling putfilep twice to ensure reference counting
               * for filep remains consistent with its state for
               * before the poll.
               */

              file_put(filep);
              file_put(filep);
            }

          if (ret < 0)
            {
              fds[i].revents |= POLLERR;
            }
        }

      /* Check if any events were posted */

      if (fds[i].revents != 0)
        {
          (*count)++;
        }

      /* Un-initialize the poll structure */

      fds[i].arg = NULL;
      fds[i].cb  = NULL;
    }
}

/****************************************************************************
 * Name: poll_setup
 *
 * Description:
 *   Setup the poll operation for each descriptor in the list.
 *
 ****************************************************************************/

static inline int poll_setup(FAR struct pollfd *fds, nfds_t nfds,
                             FAR sem_t *sem)
{
  unsigned int i;
  int ret = OK;
  int count = 0;

  /* Process each descriptor in the list */

  for (i = 0; i < nfds; i++)
    {
      /* Setup the poll descriptor
       *
       * REVISIT: In a multi-threaded environment, one use case might be to
       * share a single, array of struct pollfd in poll() calls on different
       * threads.  That use case is not supportable here because the non-
       * standard internal fields get reset here on each call to poll()
       * on each thread.
       */

      fds[i].arg     = sem;
      fds[i].cb      = poll_default_cb;
      fds[i].revents = 0;
      fds[i].priv    = NULL;

      /* Check for invalid descriptors. "If the value of fd is less than 0,
       * events shall be ignored, and revents shall be set to 0 in that entry
       * on return from poll()."
       *
       * NOTE:  There is a potential problem here.  If there is only one fd
       * and if it is negative, then poll will hang.  From my reading of the
       * spec, that appears to be the correct behavior.
       */

      if (fds[i].fd >= 0)
        {
          FAR struct file *filep;
          int num = i;

          ret = file_get(fds[i].fd, &filep);
          if (ret < 0)
            {
              num -= 1;
            }
          else
            {
              ret = file_poll(filep, &fds[i], true);
            }

          if (ret < 0)
            {
              if (num >= 0)
                {
                  poll_teardown(fds, num, &count);
                }

              fds[i].revents |= POLLERR;
              fds[i].arg = NULL;
              fds[i].cb = NULL;
              return count + 1;
            }
          else if (fds[i].revents != 0)
            {
              count++;
            }
        }
    }

  if (count > 0)
    {
      /* If there are already events available in poll_setup,
       * we execute teardown and return immediately.
       */

      poll_teardown(fds, i, &count);
    }

  return count;
}

/****************************************************************************
 * Name: poll_cleanup
 *
 * Description:
 *   Cleanup the poll operation.
 *
 ****************************************************************************/

static void poll_cleanup(FAR void *arg)
{
  FAR struct pollfd_s *fdsinfo = (FAR struct pollfd_s *)arg;
  int count;

  poll_teardown(fdsinfo->fds, fdsinfo->nfds, &count);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: poll_default_cb
 *
 * Description:
 *   The default poll callback function, this function do the final step of
 *   poll notification.
 *
 * Input Parameters:
 *   fds - The fds
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void poll_default_cb(FAR struct pollfd *fds)
{
  int semcount = 0;
  FAR sem_t *pollsem;

  if (fds->arg != NULL)
    {
      pollsem = (FAR sem_t *)fds->arg;
      nxsem_get_value(pollsem, &semcount);
      if (semcount < 1)
        {
          nxsem_post(pollsem);
        }
    }
}

/****************************************************************************
 * Name: poll_notify
 *
 * Description:
 *   Notify the poll, this function should be called by drivers to notify
 *   the caller the poll is ready.
 *
 * Input Parameters:
 *   afds     - The fds array
 *   nfds     - Number of fds array
 *   eventset - List of events to check for activity
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

noinstrument_function
void poll_notify(FAR struct pollfd **afds, int nfds, pollevent_t eventset)
{
  int i;
  FAR struct pollfd *fds;

  DEBUGASSERT(afds != NULL && nfds >= 1);

  for (i = 0; i < nfds && eventset; i++)
    {
      fds = afds[i];
      if (fds != NULL)
        {
          /* The error event must be set in fds->revents */

          fds->revents |= eventset & (fds->events | POLLERR | POLLHUP);
          if ((fds->revents & (POLLERR | POLLHUP)) != 0)
            {
              /* Error or Hung up, clear POLLOUT event */

              fds->revents &= ~POLLOUT;
            }

          if ((fds->revents != 0 || (fds->events & POLLALWAYS) != 0) &&
              fds->cb != NULL)
            {
              finfo("Report events: %08" PRIx32 "\n", fds->revents);
              fds->cb(fds);
            }
        }
    }
}

/****************************************************************************
 * Name: file_poll
 *
 * Description:
 *   Low-level poll operation based on struct file.  This is used to
 *   support detached file.
 *
 * Input Parameters:
 *   file     File structure instance
 *   fds   - The structure describing the events to be monitored, OR NULL if
 *           this is a request to stop monitoring events.
 *   setup - true: Setup up the poll; false: Teardown the poll
 *
 * Returned Value:
 *  0: Success; Negated errno on failure
 *
 ****************************************************************************/

int file_poll(FAR struct file *filep, FAR struct pollfd *fds, bool setup)
{
  FAR struct inode *inode;
  int ret = -ENOSYS;

  DEBUGASSERT(filep != NULL);
  inode = filep->f_inode;

  if (inode != NULL)
    {
      /* Is a driver registered? Does it support the poll method?
       * If not, return -ENOSYS
       */

      if ((INODE_IS_DRIVER(inode) || INODE_IS_MQUEUE(inode) ||
          INODE_IS_SOCKET(inode) || INODE_IS_PIPE(inode)) &&
          inode->u.i_ops != NULL && inode->u.i_ops->poll != NULL)
        {
          /* Yes, it does... Setup the poll */

          ret = inode->u.i_ops->poll(filep, fds, setup);
          if (ret < 0)
            {
              ferr("poll failed:%p, setup:%d, ret:%d\n",
                   inode->u.i_ops->poll, setup, ret);
            }
        }
#ifndef CONFIG_DISABLE_MOUNTPOINT
      else if (INODE_IS_MOUNTPT(inode) && inode->u.i_mops != NULL &&
               inode->u.i_mops->poll != NULL)
        {
          ret = inode->u.i_mops->poll(filep, fds, setup);
          if (ret < 0)
            {
              ferr("poll failed:%p, setup:%d, ret:%d\n",
                   inode->u.i_ops->poll, setup, ret);
            }
        }
#endif

      /* Regular files (and block devices) are always readable and
       * writable. Open Group: "Regular files shall always poll TRUE for
       * reading and writing."
       */

      else if (INODE_IS_MOUNTPT(inode) || INODE_IS_BLOCK(inode) ||
               INODE_IS_MTD(inode))
        {
          if (setup)
            {
              poll_notify(&fds, 1, POLLIN | POLLOUT);
            }

          ret = OK;
        }
    }
  else
    {
      poll_notify(&fds, 1, POLLERR | POLLHUP);
      ret = OK;
    }

  return ret;
}

/****************************************************************************
 * Name: poll
 *
 * Description:
 *   poll() waits for one of a set of file descriptors to become ready to
 *   perform I/O.  If none of the events requested (and no error) has
 *   occurred for any of  the  file  descriptors,  then  poll() blocks until
 *   one of the events occurs.
 *
 * Input Parameters:
 *   fds  - List of structures describing file descriptors to be monitored
 *   nfds - The number of entries in the list
 *   timeout - Specifies an upper limit on the time for which poll() will
 *     block in milliseconds.  A negative value of timeout means an infinite
 *     timeout.
 *
 * Returned Value:
 *   On success, the number of structures that have non-zero revents fields.
 *   A value of 0 indicates that the call timed out and no file descriptors
 *   were ready.  On error, -1 is returned, and errno is set appropriately:
 *
 *   EBADF  - An invalid file descriptor was given in one of the sets.
 *   EFAULT - The fds address is invalid
 *   EINTR  - A signal occurred before any requested event.
 *   EINVAL - The nfds value exceeds a system limit.
 *   ENOMEM - There was no space to allocate internal data structures.
 *   ENOSYS - One or more of the drivers supporting the file descriptor
 *     does not support the poll method.
 *
 ****************************************************************************/

int poll(FAR struct pollfd *fds, nfds_t nfds, int timeout)
{
  FAR struct pollfd *kfds;
  sem_t sem;
  int count = 0;
  int ret = OK;

  DEBUGASSERT(nfds == 0 || fds != NULL);

  /* poll() is a cancellation point */

  enter_cancellation_point();

#ifdef CONFIG_BUILD_KERNEL
  /* Allocate kernel memory for the fds */

  kfds = fs_heap_malloc(nfds * sizeof(struct pollfd));
  if (!kfds)
    {
      /* Out of memory */

      ret = -ENOMEM;
      goto out_with_cancelpt;
    }

  /* Copy the user fds to neutral kernel memory */

  memcpy(kfds, fds, nfds * sizeof(struct pollfd));
#else
  /* Can use the user fds directly */

  kfds = fds;
#endif

  /* Set up the poll structure */

  nxsem_init(&sem, 0, 0);

  /* If there are already events available in poll_setup,
   * we return immediately
   */

  count = poll_setup(kfds, nfds, &sem);
  if (count == 0)
    {
      struct pollfd_s fdsinfo;

      /* Push a cancellation point onto the stack.  This will be called if
       * the thread is canceled.
       */

      fdsinfo.fds = kfds;
      fdsinfo.nfds = nfds;
      tls_cleanup_push(tls_get_info(), poll_cleanup, &fdsinfo);

      if (timeout > 0)
        {
          /* "Implementations may place limitations on the granularity of
           * timeout intervals. If the requested timeout interval requires
           * a finer granularity than the implementation supports, the
           * actual timeout interval will be rounded up to the next
           * supported value." -- opengroup.org
           *
           * Round timeout up to next full tick.
           */

          /* Either wait for either a poll event(s), for a signal to occur,
           * or for the specified timeout to elapse with no event.
           *
           * NOTE: If a poll event is pending (i.e., the semaphore has
           * already been incremented), nxsem_tickwait() will not wait, but
           * will return immediately.
           */

          ret = nxsem_tickwait(&sem, MSEC2TICK((clock_t)timeout));
          if (ret < 0)
            {
              if (ret == -ETIMEDOUT)
                {
                  /* Return zero (OK) in the event of a timeout */

                  ret = OK;
                }

              /* EINTR is the only other error expected in normal operation */
            }
        }
      else if (timeout < 0)
        {
          /* Wait for the poll event or signal with no timeout */

          ret = nxsem_wait(&sem);
        }

      /* Teardown the poll operation and get the count of events.  Zero will
       * be returned in the case of a timeout.
       *
       * Preserve ret, if negative, since it holds the result of the wait.
       */

      poll_teardown(kfds, nfds, &count);

      /* Pop the cancellation point */

      tls_cleanup_pop(tls_get_info(), 0);
    }

  nxsem_destroy(&sem);

#ifdef CONFIG_BUILD_KERNEL
  /* Copy the events back to user */

  if (ret == OK)
    {
      int i;
      for (i = 0; i < nfds; i++)
        {
          fds[i].revents = kfds[i].revents;
        }
    }

  /* Free the temporary buffer */

  fs_heap_free(kfds);

out_with_cancelpt:
#endif

  leave_cancellation_point();

  if (ret < 0)
    {
      set_errno(-ret);
      return ERROR;
    }
  else
    {
      return count;
    }
}
