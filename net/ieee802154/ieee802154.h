/****************************************************************************
 * net/ieee802154/ieee802154.h
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

#ifndef __NET_IEEE802154_IEEE802154_H
#define __NET_IEEE802154_IEEE802154_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <netpacket/ieee802154.h>

#include <nuttx/net/net.h>

#ifdef CONFIG_NET_IEEE802154

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Allocate a new IEEE 802.15.4 socket data callback */

#define ieee802154_callback_alloc(dev,conn) \
  devif_callback_alloc(dev, &conn->sconn.list, &conn->sconn.list_tail)
#define ieee802154_callback_free(dev,conn,cb) \
  devif_conn_callback_free(dev, cb, &conn->sconn.list, &conn->sconn.list_tail)

/* Memory Pools */

#define IEEE802154_POOL_PREALLOCATED  0
#define IEEE802154_POOL_DYNAMIC       1

/* Frame size */

/* This maximum size of an IEEE 802.15.4 frame.  Certain, non-standard
 * devices may exceed this value, however.
 */

#define IEEE802154_MAC_STDFRAME 127

/* Space for a two byte FCS must be reserved at the end of the frame */

#define IEEE802154_MAC_FCSSIZE  2

/* This, then, is the usable size of the frame... */

#if defined(CONFIG_NET_IEEE802154_FRAMELEN)
#  define IEEE802_MAX_FRAMELEN CONFIG_NET_IEEE802154_FRAMELEN
#else
#  define IEEE802_MAX_FRAMELEN IEEE802154_MAC_STDFRAME
#endif

#define IEEE802154_FRAMELEN (IEEE802_MAX_FRAMELEN - IEEE802154_MAC_FCSSIZE)

/****************************************************************************
 * Public Type Definitions
 ****************************************************************************/

/* Used for containing and queuing IOBs along with information about the
 * source of the frame.
 */

struct iob_s;  /* Forward reference */

struct ieee802154_container_s
{
  FAR struct ieee802154_container_s *ic_flink; /* Supports a singly linked list */
  struct ieee802154_saddr_s ic_src;            /* Source of the packet */
  FAR struct iob_s *ic_iob;                    /* Contained IOB */
  uint8_t ic_pool;                             /* See IEEE802154_POOL_* definitions */
};

/* Representation of a IEEE 802.15.4 socket connection */

struct devif_callback_s;                       /* Forward reference */

struct ieee802154_conn_s
{
  /* Common prologue of all connection structures. */

  struct socket_conn_s sconn;

  /* IEEE 802.15.4-specific content follows */

  struct ieee802154_saddr_s laddr;             /* Locally bound / source address */
  struct ieee802154_saddr_s raddr;             /* Connected remote address */
  uint8_t crefs;                               /* Reference counts on this instance */
#if CONFIG_NET_IEEE802154_BACKLOG > 0
  uint8_t backlog;                             /* Number of frames in RX queue */
#endif

  /* Queue of incoming packets */

  FAR struct ieee802154_container_s *rxhead;
  FAR struct ieee802154_container_s *rxtail;
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef __cplusplus
#  define EXTERN extern "C"
extern "C"
{
#else
#  define EXTERN extern
#endif

/* The IEEE 802.15.4 socket interface */

EXTERN const struct sock_intf_s g_ieee802154_sockif;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

struct ieee802154_data_ind_s; /* Forward reference */
struct radio_driver_s;        /* Forward reference */
struct net_driver_s;          /* Forward reference */
struct socket;                /* Forward reference */
struct sockaddr;              /* Forward reference */

/****************************************************************************
 * Name: ieee802154_initialize()
 *
 * Description:
 *   Initialize the IEEE 802.15.4 socket support.  Called once and only
 *   from the network initialization logic.
 *
 * Assumptions:
 *   Called early in the initialization sequence
 *
 ****************************************************************************/

void ieee802154_initialize(void);

/****************************************************************************
 * Name: ieee802154_conn_alloc()
 *
 * Description:
 *   Allocate a new, uninitialized IEEE 802.15.4 socket connection
 *   structure. This is normally something done by the implementation of
 *   the socket() API
 *
 ****************************************************************************/

FAR struct ieee802154_conn_s *ieee802154_conn_alloc(void);

/****************************************************************************
 * Name: ieee802154_conn_free()
 *
 * Description:
 *   Free a IEEE 802.15.4 socket connection structure that is no longer in
 *   use.  This should be done by the implementation of close().
 *
 ****************************************************************************/

void ieee802154_conn_free(FAR struct ieee802154_conn_s *conn);

/****************************************************************************
 * Name: ieee802154_conn_active()
 *
 * Description:
 *   Find a connection structure that is the appropriate
 *   connection to be used with the provided Ethernet header
 *
 * Assumptions:
 *   This function is called from network logic at with the network locked.
 *
 ****************************************************************************/

FAR struct ieee802154_conn_s *
  ieee802154_conn_active(FAR const struct ieee802154_data_ind_s *meta);

/****************************************************************************
 * Name: ieee802154_conn_next()
 *
 * Description:
 *   Traverse the list of allocated IEEE 802.15.4 connections
 *
 * Assumptions:
 *   This function is called from network logic at with the network locked.
 *
 ****************************************************************************/

FAR struct ieee802154_conn_s *
  ieee802154_conn_next(FAR struct ieee802154_conn_s *conn);

/****************************************************************************
 * Name: ieee802154_input
 *
 * Description:
 *   Handle incoming IEEE 802.15.4 input
 *
 *   This function is called when the radio device driver has received an
 *   frame from the network.  The frame from the device driver must be
 *   provided in by the IOB frame argument of the  function call:
 *
 *   - The frame data is in the IOB io_data[] buffer,
 *   - The length of the frame is in the IOB io_len field, and
 *   - The offset past and radio MAC header is provided in the io_offset
 *     field.
 *
 *   The frame argument may refer to a single frame (a list of length one)
 *   or may it be the head of a list of multiple frames.
 *
 *   - The io_flink field points to the next frame in the list (if enable)
 *   - The last frame in the list will have io_flink == NULL.
 *
 * Input Parameters:
 *   radio       The radio network driver interface.
 *   framelist - The head of an incoming list of frames.  Normally this
 *               would be a single frame.  A list may be provided if
 *               appropriate, however.
 *   meta      - Meta data characterizing the received frame.
 *
 *               If there are multiple frames in the list, this metadata
 *               must apply to all of the frames in the list.
 *
 * Returned Value:
 *   OK    The IEEE 802.15.4 has been processed  and can be deleted
 *   ERROR Hold the IEEE 802.15.4 and try again later. There is a listening
 *         socket but no recv in place to catch the IEEE 802.15.4 yet.
 *         Useful when a packet arrives before a recv call is in place.
 *
 * Assumptions:
 *   Called from the network diver with the network locked.
 *
 ****************************************************************************/

/* ieee802154_input() is prototyped in include/nuttx/net/ieee802154.h */

/****************************************************************************
 * Name: ieee802154_callback
 *
 * Description:
 *   Inform the application holding the IEEE 802.15.4 socket of a change in
 *   state.
 *
 * Returned Value:
 *   OK if IEEE 802.15.4 has been processed, otherwise ERROR.
 *
 * Assumptions:
 *   This function is called from network logic at with the network locked.
 *
 ****************************************************************************/

uint16_t ieee802154_callback(FAR struct radio_driver_s *radio,
                             FAR struct ieee802154_conn_s *conn,
                             uint16_t flags);

/****************************************************************************
 * Name: ieee802154_recvmsg
 *
 * Description:
 *   Implements the socket recvfrom interface for the case of the AF_INET
 *   and AF_INET6 address families.  ieee802154_recvmsg() receives messages
 *   from a socket, and may be used to receive data on a socket whether or
 *   not it is connection-oriented.
 *
 *   If msg_name is not NULL, and the underlying protocol provides the source
 *   address, this source address is filled in. The argument 'msg_namelen' is
 *   initialized to the size of the buffer associated with msg_name, and
 *   modified on return to indicate the actual size of the address stored
 *   there.
 *
 * Input Parameters:
 *   psock    A pointer to a NuttX-specific, internal socket structure
 *   msg      Buffer to receive the message
 *   flags    Receive flags
 *
 * Returned Value:
 *   On success, returns the number of characters received. If no data is
 *   available to be received and the peer has performed an orderly shutdown,
 *   recvmsg() will return 0.  Otherwise, on errors, a negated errno value is
 *   returned (see recvmsg() for the list of appropriate error values).
 *
 ****************************************************************************/

ssize_t ieee802154_recvmsg(FAR struct socket *psock, FAR struct msghdr *msg,
                           int flags);

/****************************************************************************
 * Name: ieee802154_find_device
 *
 * Description:
 *   Select the network driver to use with the IEEE802154 transaction.
 *
 * Input Parameters:
 *   conn - IEEE802154 connection structure (not currently used).
 *   addr - The address to match the devices assigned address
 *
 * Returned Value:
 *   A pointer to the network driver to use.  NULL is returned on any
 *   failure.
 *
 ****************************************************************************/

FAR struct radio_driver_s *
  ieee802154_find_device(FAR struct ieee802154_conn_s *conn,
                         FAR const struct ieee802154_saddr_s *addr);

/****************************************************************************
 * Name: ieee802154_poll
 *
 * Description:
 *   Poll a IEEE 802.15.4 "connection" structure for availability of TX data
 *
 * Input Parameters:
 *   dev - The device driver structure to use in the send operation
 *   conn - The IEEE 802.15.4 "connection" to poll for TX data
 *
 * Returned Value:
 *   None
 *
 * Assumptions:
 *   Called from the network device interface (devif) with the network
 *   locked.
 *
 ****************************************************************************/

void ieee802154_poll(FAR struct net_driver_s *dev,
                     FAR struct ieee802154_conn_s *conn);

/****************************************************************************
 * Name: ieee802154_sendmsg
 *
 * Description:
 *   If sendmsg() is used on a connection-mode (SOCK_STREAM, SOCK_SEQPACKET)
 *   socket, the parameters 'msg_name' and 'msg_namelen' are ignored (and the
 *   error EISCONN may be returned when they are not NULL and 0), and the
 *   error ENOTCONN is returned when the socket was not actually connected.
 *
 * Input Parameters:
 *   psock    An instance of the internal socket structure.
 *   msg      Message to send
 *   flags    Send flags
 *
 * Returned Value:
 *   On success, returns the number of characters sent.  On error, a negated
 *   errno value is returned (see sendmsg() for the complete list of return
 *   values.
 *
 ****************************************************************************/

ssize_t ieee802154_sendmsg(FAR struct socket *psock, FAR struct msghdr *msg,
                           int flags);

/****************************************************************************
 * Name: ieee802154_container_initialize
 *
 * Description:
 *   This function initializes the container allocator.  This function must
 *   be called early in the initialization sequence before any socket
 *   activity.
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   None
 *
 * Assumptions:
 *   Called early in the initialization sequence
 *
 ****************************************************************************/

void ieee802154_container_initialize(void);

/****************************************************************************
 * Name: ieee802154_container_allocate
 *
 * Description:
 *   The ieee802154_container_allocate function will get a free container
 *   for use by the recvfrom() logic.
 *
 *   This function will first attempt to allocate from the g_free_container
 *   list.  If that the list is empty, then the meta-data structure will be
 *   allocated from the dynamic memory pool.
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   A reference to the allocated container structure.  All user fields in
 *   this structure have been zeroed.  On a failure to allocate, NULL is
 *   returned.
 *
 * Assumptions:
 *   The caller has locked the network.
 *
 ****************************************************************************/

FAR struct ieee802154_container_s *ieee802154_container_allocate(void);

/****************************************************************************
 * Name: ieee802154_container_free
 *
 * Description:
 *   The ieee802154_container_free function will return a container structure
 *   to the free list of containers if it was a pre-allocated container
 *   structure. If the container structure was allocated dynamically it will
 *   be deallocated.
 *
 * Input Parameters:
 *   container - container structure to free
 *
 * Returned Value:
 *   None
 *
 * Assumptions:
 *   The caller has locked the network.
 *
 ****************************************************************************/

void ieee802154_container_free(FAR struct ieee802154_container_s *container);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* CONFIG_NET_IEEE802154 */
#endif /* __NET_IEEE802154_IEEE802154_H */
