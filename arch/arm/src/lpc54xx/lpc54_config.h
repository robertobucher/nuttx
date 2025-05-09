/****************************************************************************
 * arch/arm/src/lpc54xx/lpc54_config.h
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

#ifndef __ARCH_ARM_SRC_LPC54XX_LPC54_CONFIG_H
#define __ARCH_ARM_SRC_LPC54XX_LPC54_CONFIG_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>

#include "chip.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/* Make sure that no unsupported USART, I2C master, or SPI master peripherals
 * are enabled.
 */

#ifndef CONFIG_LPC54_FLEXCOMM0
#  undef CONFIG_LPC54_I2C0_MASTER
#  undef CONFIG_LPC54_SPI0_MASTER
#  undef CONFIG_LPC54_USART0
#endif
#ifndef CONFIG_LPC54_FLEXCOMM1
#  undef CONFIG_LPC54_I2C1_MASTER
#  undef CONFIG_LPC54_SPI1_MASTER
#  undef CONFIG_LPC54_USART1
#endif
#ifndef CONFIG_LPC54_FLEXCOMM2
#  undef CONFIG_LPC54_I2C2_MASTER
#  undef CONFIG_LPC54_SPI2_MASTER
#  undef CONFIG_LPC54_USART2
#endif
#ifndef CONFIG_LPC54_FLEXCOMM3
#  undef CONFIG_LPC54_I2C3_MASTER
#  undef CONFIG_LPC54_SPI3_MASTER
#  undef CONFIG_LPC54_USART3
#endif
#ifndef CONFIG_LPC54_FLEXCOMM4
#  undef CONFIG_LPC54_I2C4_MASTER
#  undef CONFIG_LPC54_SPI4_MASTER
#  undef CONFIG_LPC54_USART4
#endif
#ifndef CONFIG_LPC54_FLEXCOMM5
#  undef CONFIG_LPC54_I2C5_MASTER
#  undef CONFIG_LPC54_SPI5_MASTER
#  undef CONFIG_LPC54_USART5
#endif
#ifndef CONFIG_LPC54_FLEXCOMM6
#  undef CONFIG_LPC54_I2C6_MASTER
#  undef CONFIG_LPC54_SPI6_MASTER
#  undef CONFIG_LPC54_USART6
#endif
#ifndef CONFIG_LPC54_FLEXCOMM7
#  undef CONFIG_LPC54_I2C7_MASTER
#  undef CONFIG_LPC54_SPI7_MASTER
#  undef CONFIG_LPC54_USART7
#endif
#ifndef CONFIG_LPC54_FLEXCOMM8
#  undef CONFIG_LPC54_I2C8_MASTER
#  undef CONFIG_LPC54_SPI8_MASTER
#  undef CONFIG_LPC54_USART8
#endif
#ifndef CONFIG_LPC54_FLEXCOMM9
#  undef CONFIG_LPC54_I2C9_MASTER
#  undef CONFIG_LPC54_SPI9_MASTER
#  undef CONFIG_LPC54_USART9
#endif

#ifdef CONFIG_LPC54_I2C0_MASTER
#  undef CONFIG_LPC54_SPI0_MASTER
#  undef CONFIG_LPC54_USART0
#endif
#ifdef CONFIG_LPC54_I2C1_MASTER
#  undef CONFIG_LPC54_SPI1_MASTER
#  undef CONFIG_LPC54_USART1
#endif
#ifdef CONFIG_LPC54_I2C2_MASTER
#  undef CONFIG_LPC54_SPI2_MASTER
#  undef CONFIG_LPC54_USART2
#endif
#ifdef CONFIG_LPC54_I2C3_MASTER
#  undef CONFIG_LPC54_SPI3_MASTER
#  undef CONFIG_LPC54_USART3
#endif
#ifdef CONFIG_LPC54_I2C4_MASTER
#  undef CONFIG_LPC54_SPI4_MASTER
#  undef CONFIG_LPC54_USART4
#endif
#ifdef CONFIG_LPC54_I2C5_MASTER
#  undef CONFIG_LPC54_SPI5_MASTER
#  undef CONFIG_LPC54_USART5
#endif
#ifdef CONFIG_LPC54_I2C6_MASTER
#  undef CONFIG_LPC54_SPI6_MASTER
#  undef CONFIG_LPC54_USART6
#endif
#ifdef CONFIG_LPC54_I2C7_MASTER
#  undef CONFIG_LPC54_SPI7_MASTER
#  undef CONFIG_LPC54_USART7
#endif
#ifdef CONFIG_LPC54_I2C8_MASTER
#  undef CONFIG_LPC54_SPI8_MASTER
#  undef CONFIG_LPC54_USART8
#endif
#ifdef CONFIG_LPC54_I2C9_MASTER
#  undef CONFIG_LPC54_SPI9_MASTER
#  undef CONFIG_LPC54_USART9
#endif

#ifdef CONFIG_LPC54_SPI0_MASTER
#  undef CONFIG_LPC54_USART0
#endif
#ifdef CONFIG_LPC54_SPI1_MASTER
#  undef CONFIG_LPC54_USART1
#endif
#ifdef CONFIG_LPC54_SPI2_MASTER
#  undef CONFIG_LPC54_USART2
#endif
#ifdef CONFIG_LPC54_SPI3_MASTER
#  undef CONFIG_LPC54_USART3
#endif
#ifdef CONFIG_LPC54_SPI4_MASTER
#  undef CONFIG_LPC54_USART4
#endif
#ifdef CONFIG_LPC54_SPI5_MASTER
#  undef CONFIG_LPC54_USART5
#endif
#ifdef CONFIG_LPC54_SPI6_MASTER
#  undef CONFIG_LPC54_USART6
#endif
#ifdef CONFIG_LPC54_SPI7_MASTER
#  undef CONFIG_LPC54_USART7
#endif
#ifdef CONFIG_LPC54_SPI8_MASTER
#  undef CONFIG_LPC54_USART8
#endif
#ifdef CONFIG_LPC54_SPI9_MASTER
#  undef CONFIG_LPC54_USART9
#endif

/* Check if we have an I2C device */

#undef CONFIG_LPC54_HAVE_I2C_MASTER
#undef HAVE_I2C_MASTER_DEVICE

#if defined(CONFIG_LPC54_I2C0_MASTER) || defined(CONFIG_LPC54_I2C1_MASTER) || \
    defined(CONFIG_LPC54_I2C2_MASTER) || defined(CONFIG_LPC54_I2C3_MASTER) || \
    defined(CONFIG_LPC54_I2C4_MASTER) || defined(CONFIG_LPC54_I2C5_MASTER) || \
    defined(CONFIG_LPC54_I2C6_MASTER) || defined(CONFIG_LPC54_I2C7_MASTER) || \
    defined(CONFIG_LPC54_I2C8_MASTER) || defined(CONFIG_LPC54_I2C9_MASTER)
#  define HAVE_I2C_MASTER_DEVICE 1
#endif

/* Check if we have an SPI device */

#undef CONFIG_LPC54_HAVE_SPI_MASTER
#undef HAVE_SP_MASTERI_DEVICE

#if defined(CONFIG_LPC54_SPI0_MASTER) || defined(CONFIG_LPC54_SPI1_MASTER) || \
    defined(CONFIG_LPC54_SPI2_MASTER) || defined(CONFIG_LPC54_SPI3_MASTER) || \
    defined(CONFIG_LPC54_SPI4_MASTER) || defined(CONFIG_LPC54_SPI5_MASTER) || \
    defined(CONFIG_LPC54_SPI6_MASTER) || defined(CONFIG_LPC54_SPI7_MASTER) || \
    defined(CONFIG_LPC54_SPI8_MASTER) || defined(CONFIG_LPC54_SPI9_MASTER)
#  define HAVE_SPI_MASTER_DEVICE 1
#endif

/* Map logical USART names (Just for simplicity of naming) */

#undef HAVE_USART0
#undef HAVE_USART1
#undef HAVE_USART2
#undef HAVE_USART3
#undef HAVE_USART4
#undef HAVE_USART5
#undef HAVE_USART6
#undef HAVE_USART7
#undef HAVE_USART8
#undef HAVE_USART9

#ifdef CONFIG_LPC54_USART0
#  define HAVE_USART0 1
#endif
#ifdef CONFIG_LPC54_USART1
#  define HAVE_USART1 1
#endif
#ifdef CONFIG_LPC54_USART2
#  define HAVE_USART2 1
#endif
#ifdef CONFIG_LPC54_USART3
#  define HAVE_USART3 1
#endif
#ifdef CONFIG_LPC54_USART4
#  define HAVE_USART4 1
#endif
#ifdef CONFIG_LPC54_USART5
#  define HAVE_USART5 1
#endif
#ifdef CONFIG_LPC54_USART6
#  define HAVE_USART6 1
#endif
#ifdef CONFIG_LPC54_USART7
#  define HAVE_USART7 1
#endif
#ifdef CONFIG_LPC54_USART8
#  define HAVE_USART8 1
#endif
#ifdef CONFIG_LPC54_USART9
#  define HAVE_USART9 1
#endif

/* Check if we have a USART device */

#undef CONFIG_LPC54_HAVE_USART
#undef HAVE_USART_DEVICE

#if defined(HAVE_USART0) || defined(HAVE_USART1) || defined(HAVE_USART2) || \
    defined(HAVE_USART3) || defined(HAVE_USART4) || defined(HAVE_USART5) || \
    defined(HAVE_USART6) || defined(HAVE_USART7) || defined(HAVE_USART8) || \
    defined(HAVE_USART9)
#  define HAVE_USART_DEVICE 1
#endif

/* Is there a serial console? There should be at most one defined.
 *  It could be on any USARTn, n=0,1,2,3,4,5
 */

#undef HAVE_USART_CONSOLE

#if defined(CONFIG_USART0_SERIAL_CONSOLE) && defined(HAVE_USART0)
#  undef CONFIG_USART1_SERIAL_CONSOLE
#  undef CONFIG_USART2_SERIAL_CONSOLE
#  undef CONFIG_USART3_SERIAL_CONSOLE
#  undef CONFIG_USART4_SERIAL_CONSOLE
#  undef CONFIG_USART5_SERIAL_CONSOLE
#  undef CONFIG_USART6_SERIAL_CONSOLE
#  undef CONFIG_USART7_SERIAL_CONSOLE
#  undef CONFIG_USART8_SERIAL_CONSOLE
#  undef CONFIG_USART9_SERIAL_CONSOLE
#  define HAVE_USART_CONSOLE 1
#elif defined(CONFIG_USART1_SERIAL_CONSOLE) && defined(HAVE_USART1)
#  undef CONFIG_USART0_SERIAL_CONSOLE
#  undef CONFIG_USART2_SERIAL_CONSOLE
#  undef CONFIG_USART3_SERIAL_CONSOLE
#  undef CONFIG_USART4_SERIAL_CONSOLE
#  undef CONFIG_USART5_SERIAL_CONSOLE
#  undef CONFIG_USART6_SERIAL_CONSOLE
#  undef CONFIG_USART7_SERIAL_CONSOLE
#  undef CONFIG_USART8_SERIAL_CONSOLE
#  undef CONFIG_USART9_SERIAL_CONSOLE
#  define HAVE_USART_CONSOLE 1
#elif defined(CONFIG_USART2_SERIAL_CONSOLE) && defined(HAVE_USART2)
#  undef CONFIG_USART0_SERIAL_CONSOLE
#  undef CONFIG_USART1_SERIAL_CONSOLE
#  undef CONFIG_USART3_SERIAL_CONSOLE
#  undef CONFIG_USART4_SERIAL_CONSOLE
#  undef CONFIG_USART5_SERIAL_CONSOLE
#  undef CONFIG_USART6_SERIAL_CONSOLE
#  undef CONFIG_USART7_SERIAL_CONSOLE
#  undef CONFIG_USART8_SERIAL_CONSOLE
#  undef CONFIG_USART9_SERIAL_CONSOLE
#  define HAVE_USART_CONSOLE 1
#elif defined(CONFIG_USART3_SERIAL_CONSOLE) && defined(HAVE_USART3)
#  undef CONFIG_USART0_SERIAL_CONSOLE
#  undef CONFIG_USART1_SERIAL_CONSOLE
#  undef CONFIG_USART2_SERIAL_CONSOLE
#  undef CONFIG_USART4_SERIAL_CONSOLE
#  undef CONFIG_USART5_SERIAL_CONSOLE
#  undef CONFIG_USART6_SERIAL_CONSOLE
#  undef CONFIG_USART7_SERIAL_CONSOLE
#  undef CONFIG_USART8_SERIAL_CONSOLE
#  undef CONFIG_USART9_SERIAL_CONSOLE
#  define HAVE_USART_CONSOLE 1
#elif defined(CONFIG_USART4_SERIAL_CONSOLE) && defined(HAVE_USART4)
#  undef CONFIG_USART0_SERIAL_CONSOLE
#  undef CONFIG_USART1_SERIAL_CONSOLE
#  undef CONFIG_USART2_SERIAL_CONSOLE
#  undef CONFIG_USART3_SERIAL_CONSOLE
#  undef CONFIG_USART5_SERIAL_CONSOLE
#  undef CONFIG_USART6_SERIAL_CONSOLE
#  undef CONFIG_USART7_SERIAL_CONSOLE
#  undef CONFIG_USART8_SERIAL_CONSOLE
#  undef CONFIG_USART9_SERIAL_CONSOLE
#  define HAVE_USART_CONSOLE 1
#elif defined(CONFIG_USART5_SERIAL_CONSOLE) && defined(HAVE_USART5)
#  undef CONFIG_USART0_SERIAL_CONSOLE
#  undef CONFIG_USART1_SERIAL_CONSOLE
#  undef CONFIG_USART2_SERIAL_CONSOLE
#  undef CONFIG_USART3_SERIAL_CONSOLE
#  undef CONFIG_USART4_SERIAL_CONSOLE
#  undef CONFIG_USART6_SERIAL_CONSOLE
#  undef CONFIG_USART7_SERIAL_CONSOLE
#  undef CONFIG_USART8_SERIAL_CONSOLE
#  undef CONFIG_USART9_SERIAL_CONSOLE
#  define HAVE_USART_CONSOLE 1
#elif defined(CONFIG_USART6_SERIAL_CONSOLE) && defined(HAVE_USART6)
#  undef CONFIG_USART0_SERIAL_CONSOLE
#  undef CONFIG_USART1_SERIAL_CONSOLE
#  undef CONFIG_USART2_SERIAL_CONSOLE
#  undef CONFIG_USART3_SERIAL_CONSOLE
#  undef CONFIG_USART4_SERIAL_CONSOLE
#  undef CONFIG_USART5_SERIAL_CONSOLE
#  undef CONFIG_USART7_SERIAL_CONSOLE
#  undef CONFIG_USART8_SERIAL_CONSOLE
#  undef CONFIG_USART9_SERIAL_CONSOLE
#  define HAVE_USART_CONSOLE 1
#elif defined(CONFIG_USART7_SERIAL_CONSOLE) && defined(HAVE_USART7)
#  undef CONFIG_USART0_SERIAL_CONSOLE
#  undef CONFIG_USART1_SERIAL_CONSOLE
#  undef CONFIG_USART2_SERIAL_CONSOLE
#  undef CONFIG_USART3_SERIAL_CONSOLE
#  undef CONFIG_USART4_SERIAL_CONSOLE
#  undef CONFIG_USART5_SERIAL_CONSOLE
#  undef CONFIG_USART6_SERIAL_CONSOLE
#  undef CONFIG_USART8_SERIAL_CONSOLE
#  undef CONFIG_USART9_SERIAL_CONSOLE
#  define HAVE_USART_CONSOLE 1
#elif defined(CONFIG_USART8_SERIAL_CONSOLE) && defined(HAVE_USART8)
#  undef CONFIG_USART0_SERIAL_CONSOLE
#  undef CONFIG_USART1_SERIAL_CONSOLE
#  undef CONFIG_USART2_SERIAL_CONSOLE
#  undef CONFIG_USART3_SERIAL_CONSOLE
#  undef CONFIG_USART4_SERIAL_CONSOLE
#  undef CONFIG_USART5_SERIAL_CONSOLE
#  undef CONFIG_USART6_SERIAL_CONSOLE
#  undef CONFIG_USART7_SERIAL_CONSOLE
#  undef CONFIG_USART9_SERIAL_CONSOLE
#  define HAVE_USART_CONSOLE 1
#elif defined(CONFIG_USART9_SERIAL_CONSOLE) && defined(HAVE_USART9)
#  undef CONFIG_USART0_SERIAL_CONSOLE
#  undef CONFIG_USART1_SERIAL_CONSOLE
#  undef CONFIG_USART2_SERIAL_CONSOLE
#  undef CONFIG_USART3_SERIAL_CONSOLE
#  undef CONFIG_USART4_SERIAL_CONSOLE
#  undef CONFIG_USART5_SERIAL_CONSOLE
#  undef CONFIG_USART6_SERIAL_CONSOLE
#  undef CONFIG_USART7_SERIAL_CONSOLE
#  undef CONFIG_USART8_SERIAL_CONSOLE
#  define HAVE_USART_CONSOLE 1
#else
#  ifdef CONFIG_DEV_CONSOLE
#    warning "No valid CONFIG_[LP]USART[n]_SERIAL_CONSOLE Setting"
#  endif
#  undef CONFIG_USART0_SERIAL_CONSOLE
#  undef CONFIG_USART1_SERIAL_CONSOLE
#  undef CONFIG_USART2_SERIAL_CONSOLE
#  undef CONFIG_USART3_SERIAL_CONSOLE
#  undef CONFIG_USART4_SERIAL_CONSOLE
#  undef CONFIG_USART5_SERIAL_CONSOLE
#  undef CONFIG_USART6_SERIAL_CONSOLE
#  undef CONFIG_USART7_SERIAL_CONSOLE
#  undef CONFIG_USART8_SERIAL_CONSOLE
#  undef CONFIG_USART9_SERIAL_CONSOLE
#endif

/* Check USART flow control (Not yet supported) */

# undef CONFIG_USART0_FLOWCONTROL
# undef CONFIG_USART1_FLOWCONTROL
# undef CONFIG_USART2_FLOWCONTROL
# undef CONFIG_USART3_FLOWCONTROL
# undef CONFIG_USART4_FLOWCONTROL
# undef CONFIG_USART5_FLOWCONTROL
# undef CONFIG_USART6_FLOWCONTROL
# undef CONFIG_USART7_FLOWCONTROL
# undef CONFIG_USART8_FLOWCONTROL
# undef CONFIG_USART9_FLOWCONTROL

/****************************************************************************
 * Public Functions Prototypes
 ****************************************************************************/

#endif /* __ARCH_ARM_SRC_LPC54XX_LPC54_CONFIG_H */
