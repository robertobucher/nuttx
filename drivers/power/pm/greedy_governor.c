/****************************************************************************
 * drivers/power/pm/greedy_governor.c
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
#include <stdbool.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <debug.h>
#include <assert.h>

#include <nuttx/kmalloc.h>
#include <nuttx/signal.h>
#include <nuttx/fs/fs.h>
#include <nuttx/power/pm.h>
#include <nuttx/power/power_ioctl.h>

#include <nuttx/irq.h>

#include "pm.h"

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/* PM governor methods */

static void greedy_governor_statechanged(int domain,
                                         enum pm_state_e newstate);
static enum pm_state_e greedy_governor_checkstate(int domain);
static void greedy_governor_activity(int domain, int count);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct pm_governor_s g_greedy_governor_ops =
{
  NULL,                         /* initialize */
  NULL,                         /* deinitialize */
  greedy_governor_statechanged, /* statechanged */
  greedy_governor_checkstate,   /* checkstate */
  greedy_governor_activity,     /* activity */
  NULL                          /* priv */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: greedy_governor_statechanged
 ****************************************************************************/

static void greedy_governor_statechanged(int domain,
                                         enum pm_state_e newstate)
{
  /* no need to react to state changes */

  UNUSED(domain);
  UNUSED(newstate);
}

/****************************************************************************
 * Name: user_governor_checkstate
 ****************************************************************************/

static enum pm_state_e greedy_governor_checkstate(int domain)
{
  FAR struct pm_domain_s *pdom;
  irqstate_t flags;
  int state;

  pdom = &g_pmdomains[domain];
  state = PM_NORMAL;

  /* We disable interrupts since pm_stay()/pm_relax() could be simultaneously
   * invoked, which modifies the stay count which we are about to read
   */

  flags = spin_lock_irqsave(&pdom->lock);

  /* Find the lowest power-level which is not locked. */

  while (dq_empty(&pdom->wakelock[state]) && state < (PM_COUNT - 1))
    {
      state++;
    }

  spin_unlock_irqrestore(&pdom->lock, flags);

  /* Return the found state */

  return state;
}

/****************************************************************************
 * Name: greedy_activity
 ****************************************************************************/

static void greedy_governor_activity(int domain, int count)
{
  pm_staytimeout(domain, PM_NORMAL, (count ? count : 1) * 1000);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: pm_greedy_governor_initialize
 *
 * Description:
 *   Register the user_governor driver as the specified device.
 *
 * Returned Value:
 *   Zero (OK) is returned on success.  Otherwise a negated errno value is
 *   returned to indicate the nature of the failure.
 *
 ****************************************************************************/

FAR const struct pm_governor_s *pm_greedy_governor_initialize(void)
{
  return &g_greedy_governor_ops;
}
