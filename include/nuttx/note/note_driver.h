/****************************************************************************
 * include/nuttx/note/note_driver.h
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

#ifndef __INCLUDE_NUTTX_NOTE_NOTE_DRIVER_H
#define __INCLUDE_NUTTX_NOTE_NOTE_DRIVER_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <nuttx/sched.h>
#include <nuttx/sched_note.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct note_driver_s;

struct note_driver_ops_s
{
  CODE void (*add)(FAR struct note_driver_s *drv,
                   FAR const void *note, size_t notelen);
  CODE void (*start)(FAR struct note_driver_s *drv, FAR struct tcb_s *tcb);
  CODE void (*stop)(FAR struct note_driver_s *drv, FAR struct tcb_s *tcb);
#ifdef CONFIG_SCHED_INSTRUMENTATION_SWITCH
  CODE void (*suspend)(FAR struct note_driver_s *drv, FAR struct tcb_s *tcb);
  CODE void (*resume)(FAR struct note_driver_s *drv, FAR struct tcb_s *tcb);
#endif
#ifdef CONFIG_SMP
  CODE void (*cpu_start)(FAR struct note_driver_s *drv,
                         FAR struct tcb_s *tcb, int cpu);
  CODE void (*cpu_started)(FAR struct note_driver_s *drv,
                           FAR struct tcb_s *tcb);
#  ifdef CONFIG_SCHED_INSTRUMENTATION_SWITCH
  CODE void (*cpu_pause)(FAR struct note_driver_s *drv,
                         FAR struct tcb_s *tcb, int cpu);
  CODE void (*cpu_paused)(FAR struct note_driver_s *drv,
                          FAR struct tcb_s *tcb);
  CODE void (*cpu_resume)(FAR struct note_driver_s *drv,
                          FAR struct tcb_s *tcb, int cpu);
  CODE void (*cpu_resumed)(FAR struct note_driver_s *drv,
                           FAR struct tcb_s *tcb);
#  endif
#endif
#ifdef CONFIG_SCHED_INSTRUMENTATION_PREEMPTION
  CODE void (*preemption)(FAR struct note_driver_s *drv,
                          FAR struct tcb_s *tcb, bool locked);
#endif
#ifdef CONFIG_SCHED_INSTRUMENTATION_CSECTION
  CODE void (*csection)(FAR struct note_driver_s *drv,
                        FAR struct tcb_s *tcb, bool enter);
#endif
#ifdef CONFIG_SCHED_INSTRUMENTATION_SPINLOCKS
  CODE void (*spinlock)(FAR struct note_driver_s *drv, FAR struct tcb_s *tcb,
                        FAR volatile void *spinlock, int type);
#endif
#ifdef CONFIG_SCHED_INSTRUMENTATION_SYSCALL
  CODE void (*syscall_enter)(FAR struct note_driver_s *drv,
                             int nr, int argc, va_list *ap);
  CODE void (*syscall_leave)(FAR struct note_driver_s *drv,
                             int nr, uintptr_t result);
#endif
#ifdef CONFIG_SCHED_INSTRUMENTATION_IRQHANDLER
  CODE void (*irqhandler)(FAR struct note_driver_s *drv, int irq,
                          FAR void *handler, bool enter);
#endif
#ifdef CONFIG_SCHED_INSTRUMENTATION_WDOG
  CODE void (*wdog)(FAR struct note_driver_s *drv, uint8_t event,
                    FAR void *handler, FAR const void *arg);
#endif
#ifdef CONFIG_SCHED_INSTRUMENTATION_HEAP
  CODE void (*heap)(FAR struct note_driver_s *drv, uint8_t event,
                    FAR void *heap, FAR void *mem, size_t size,
                    size_t curused);
#endif
#ifdef CONFIG_SCHED_INSTRUMENTATION_DUMP
  CODE void (*event)(FAR struct note_driver_s *drv, uintptr_t ip,
                     uint8_t event, FAR const void *buf, size_t len);
  CODE void (*vprintf)(FAR struct note_driver_s *drv, uintptr_t ip,
                       FAR const char *fmt, va_list va) printf_like(3, 0);
#endif
};

#ifdef CONFIG_SCHED_INSTRUMENTATION_FILTER
struct note_filter_s
{
  struct note_filter_mode_s mode;
#  ifdef CONFIG_SCHED_INSTRUMENTATION_DUMP
  struct note_filter_tag_s tag_mask;
#  endif
#  ifdef CONFIG_SCHED_INSTRUMENTATION_IRQHANDLER
  struct note_filter_irq_s irq_mask;
#  endif
#  ifdef CONFIG_SCHED_INSTRUMENTATION_SYSCALL
  struct note_filter_syscall_s syscall_mask;
#  endif
};
#endif

struct note_driver_s
{
#ifdef CONFIG_SCHED_INSTRUMENTATION_FILTER
  FAR const char *name;
  struct note_filter_s filter;
#endif
  FAR const struct note_driver_ops_s *ops;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#if defined(__KERNEL__) || defined(CONFIG_BUILD_FLAT)

#ifdef CONFIG_DRIVERS_NOTE

/****************************************************************************
 * Name: note_early_initialize
 *
 * Description:
 *   Early register sched note related drivers that do not rely on system
 *   features like mm.
 *
 * Input Parameters:
 *   None.
 *
 * Returned Value:
 *   Zero on success. A negative errno value is returned on a failure.
 *
 ****************************************************************************/

int note_early_initialize(void);

/****************************************************************************
 * Name: note_initialize
 *
 * Description:
 *   Register sched note related drivers at /dev folder that can be used by
 *   an application to read or filter the note data.
 *
 * Input Parameters:
 *   None.
 *
 * Returned Value:
 *   Zero on success. A negative errno value is returned on a failure.
 *
 ****************************************************************************/

int note_initialize(void);
#endif

#endif /* defined(__KERNEL__) || defined(CONFIG_BUILD_FLAT) */

#if defined(CONFIG_DRIVERS_NOTE_TASKNAME_BUFSIZE) && \
    CONFIG_DRIVERS_NOTE_TASKNAME_BUFSIZE > 0

/****************************************************************************
 * Name: note_get_taskname
 *
 * Description:
 *   Get the task name string of the specified PID
 *
 * Input Parameters:
 *   PID - Task ID
 *
 * Returned Value:
 *   Return name if task name can be retrieved, otherwise NULL
 *
 ****************************************************************************/

FAR const char *note_get_taskname(pid_t pid);

#endif /* defined(CONFIG_DRIVERS_NOTE_TASKNAME_BUFSIZE) && \
        * CONFIG_DRIVERS_NOTE_TASKNAME_BUFSIZE > 0
        */

/****************************************************************************
 * Name: note_driver_register
 ****************************************************************************/

int note_driver_register(FAR struct note_driver_s *driver);

#endif /* __INCLUDE_NUTTX_NOTE_NOTE_DRIVER_H */
