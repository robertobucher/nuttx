/****************************************************************************
 * arch/renesas/src/common/renesas_internal.h
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

#ifndef ___ARCH_RENESAS_SRC_COMMON_UP_INTERNAL_H
#define ___ARCH_RENESAS_SRC_COMMON_UP_INTERNAL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#ifndef __ASSEMBLY__
#  include <nuttx/compiler.h>
#  include <stdint.h>
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Determine which (if any) console driver to use.  NOTE that the naming
 * implies that the console is a serial driver.  That is usually the case,
 * however, if no UARTs are enabled, the console could also be provided
 * through some other device, such as an LCD.  Architecture-specific logic
 * will have to detect that case.
 *
 * If a console is enabled and no other console device is specified, then
 * a serial console is assumed.
 */

#ifndef CONFIG_DEV_CONSOLE
#  undef  USE_SERIALDRIVER
#  undef  USE_EARLYSERIALINIT
#else
#  if defined(CONFIG_CONSOLE_SYSLOG)
#    undef  USE_SERIALDRIVER
#    undef  USE_EARLYSERIALINIT
#  else
#    define USE_SERIALDRIVER 1
#    define USE_EARLYSERIALINIT 1
#  endif
#endif

/* If some other device is used as the console, then the serial driver may
 * still be needed.  Let's assume that if the upper half serial driver is
 * built, then the lower half will also be needed.  There is no need for
 * the early serial initialization in this case.
 */

#if !defined(USE_SERIALDRIVER) && defined(CONFIG_STANDARD_SERIAL)
#  define USE_SERIALDRIVER 1
#endif

/* Check if an interrupt stack size is configured */

#ifndef CONFIG_ARCH_INTERRUPTSTACK
#  define CONFIG_ARCH_INTERRUPTSTACK 0
#endif

/* The SH stack must be aligned at word (4 byte) boundaries. If necessary
 * frame_size must be rounded up to the next boundary
 */

#define STACK_ALIGNMENT     4

/* Stack alignment macros */

#define STACK_ALIGN_MASK    (STACK_ALIGNMENT - 1)
#define STACK_ALIGN_DOWN(a) ((a) & ~STACK_ALIGN_MASK)
#define STACK_ALIGN_UP(a)   (((a) + STACK_ALIGN_MASK) & ~STACK_ALIGN_MASK)

#define renesas_savestate(regs)  renesas_copystate(regs, up_current_regs())

#define getreg8(a)          (*(volatile uint8_t *)(a))
#define putreg8(v,a)        (*(volatile uint8_t *)(a) = (v))
#define getreg16(a)         (*(volatile uint16_t *)(a))
#define putreg16(v,a)       (*(volatile uint16_t *)(a) = (v))
#define getreg32(a)         (*(volatile uint32_t *)(a))
#define putreg32(v,a)       (*(volatile uint32_t *)(a) = (v))

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifndef __ASSEMBLY__
typedef void (*up_vector_t)(void);
#endif

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__
/* This is the beginning of heap as provided from up_head.S.
 * This is the first address in DRAM after the loaded
 * program+bss+idle stack.  The end of the heap is
 * CONFIG_RAM_END
 */

extern uint32_t g_idle_topstack;
#endif

/* Address of the saved user stack pointer */

#ifndef __ASSEMBLY__
#  if CONFIG_ARCH_INTERRUPTSTACK > 3
     extern uint8_t g_intstackalloc[];
     extern uint8_t g_intstacktop[];
#  endif
#endif

/****************************************************************************
 * Inline Functions
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifndef __ASSEMBLY__

/* Defined in files with the same name as the function */

void renesas_copystate(uint32_t *dest, uint32_t *src);
void renesas_dataabort(uint32_t *regs);
void renesas_decodeirq(uint32_t *regs);
uint32_t *renesas_doirq(int irq, uint32_t *regs);
void renesas_fullcontextrestore(uint32_t *regs) noreturn_function;
void renesas_prefetchabort(uint32_t *regs);
void renesas_sigdeliver(void);
void renesas_syscall(uint32_t *regs);
void renesas_lowputc(char ch);
void renesas_lowputs(const char *str);

/* Defined in xyz_vectors.S */

void renesas_vectorundefinsn(void);
void renesas_vectorswi(void);
void renesas_vectorprefetch(void);
void renesas_vectordata(void);
void renesas_vectoraddrexcptn(void);
void renesas_vectorirq(void);
void renesas_vectorfiq(void);

/* Defined in xyz_serial.c */

#ifdef USE_EARLYSERIALINIT
void renesas_earlyconsoleinit(void);
#endif

#ifdef USE_SERIALDRIVER
void renesas_consoleinit(void);
void renesas_serialinit(void);
#endif

void renesas_lowputc(char ch);

/* Defined in board/xyz_lcd.c */

#ifdef CONFIG_SLCD_CONSOLE
void renesas_lcdinit(void);
void renesas_lcdputc(char ch);
#else
#  define renesas_lcdinit()
#  define renesas_lcdputc(ch)
#endif

/* Defined in board/xyz_network.c */

#if defined(CONFIG_NET) && !defined(CONFIG_NETDEV_LATEINIT)
void renesas_netinitialize(void);
#else
#  define renesas_netinitialize()
#endif

/* USB */

#ifdef CONFIG_USBDEV
void renesas_usbinitialize(void);
void renesas_usbuninitialize(void);
#else
#  define renesas_usbinitialize()
#  define renesas_usbuninitialize()
#endif

#endif /* __ASSEMBLY__ */
#endif /* ___ARCH_RENESAS_SRC_COMMON_UP_INTERNAL_H */
