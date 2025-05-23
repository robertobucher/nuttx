/****************************************************************************
 * arch/risc-v/include/irq.h
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

/* This file should never be included directly but, rather, only indirectly
 * through nuttx/irq.h
 */

#ifndef __ARCH_RISCV_INCLUDE_IRQ_H
#define __ARCH_RISCV_INCLUDE_IRQ_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

/* Include chip-specific IRQ definitions (including IRQ numbers) */

#include <nuttx/config.h>

#include <sys/types.h>

#include <arch/csr.h>
#include <arch/chip/irq.h>
#include <arch/mode.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef __ASSEMBLY__
#  define __STR(s)  s
#else
#  define __STR(s)  #s
#endif
#define __XSTR(s)   __STR(s)

/****************************************************************************
 * Map RISC-V exception code to NuttX IRQ,
 * the exception that code > 19 is reserved or custom exception.
 *
 * The content of vector table:
 *
 * |            IRQ            |              Comments              |
 * |:-------------------------:|:----------------------------------:|
 * |             0             |   Instruction Address Misaligned   |
 * |             1             |      Instruction Access Fault      |
 * |             2             |         Illegal Instruction        |
 * |            ...            |          Other exceptions          |
 * |    RISCV_MAX_EXCEPTION    |  The IRQ number of last exception  |
 * |  RISCV_MAX_EXCEPTION + 1  |  The IRQ number of first interrupt |
 * |  RISCV_MAX_EXCEPTION + 2  | The IRQ number of second interrupt |
 * | RISCV_MAX_EXCEPTION + xxx |   The IRQ number of xxx interrupt  |
 *
 * And please provide the definition of custom exception if exists:
 * #define RISCV_CUSTOM_EXCEPTION_REASONS  \
 *    "Custom exception1", \
 *    "Custom exception2",
 *
 ****************************************************************************/

/* IRQ 0-RISCV_MAX_EXCEPTION : (exception:interrupt=0) */

#define RISCV_IRQ_IAMISALIGNED  (0)   /* Instruction Address Misaligned */
#define RISCV_IRQ_IAFAULT       (1)   /* Instruction Access Fault */
#define RISCV_IRQ_IINSTRUCTION  (2)   /* Illegal Instruction */
#define RISCV_IRQ_BPOINT        (3)   /* Break Point */
#define RISCV_IRQ_LAMISALIGNED  (4)   /* Load Address Misaligned */
#define RISCV_IRQ_LAFAULT       (5)   /* Load Access Fault */
#define RISCV_IRQ_SAMISALIGNED  (6)   /* Store/AMO Address Misaligned */
#define RISCV_IRQ_SAFAULT       (7)   /* Store/AMO Access Fault */
#define RISCV_IRQ_ECALLU        (8)   /* Environment Call from U-mode */
#define RISCV_IRQ_ECALLS        (9)   /* Environment Call from S-mode */
#define RISCV_IRQ_ECALLH        (10)  /* Environment Call from H-mode */
#define RISCV_IRQ_ECALLM        (11)  /* Environment Call from M-mode */
#define RISCV_IRQ_INSTRUCTIONPF (12)  /* Instruction page fault */
#define RISCV_IRQ_LOADPF        (13)  /* Load page fault */
#define RISCV_IRQ_RESERVED14    (14)  /* Reserved */
#define RISCV_IRQ_STOREPF       (15)  /* Store/AMO page fault */
#define RISCV_IRQ_RESERVED16    (16)  /* Reserved */
#define RISCV_IRQ_RESERVED17    (17)  /* Reserved */
#define RISCV_IRQ_SOFTWARE      (18)  /* Software check */
#define RISCV_IRQ_HARDWARE      (19)  /* Hardware error */

/* Keep origin definition here for compatibility */

#ifndef RISCV_MAX_EXCEPTION
#  define RISCV_MAX_EXCEPTION   (15)
#endif

/* IRQ (RISCV_MAX_EXCEPTION + 1)- : (async event:interrupt=1) */

#define RISCV_IRQ_ASYNC         (RISCV_MAX_EXCEPTION + 1)
#define RISCV_IRQ_SSOFT         (RISCV_IRQ_ASYNC + 1)  /* Supervisor Software Int */
#define RISCV_IRQ_MSOFT         (RISCV_IRQ_ASYNC + 3)  /* Machine Software Int */
#define RISCV_IRQ_STIMER        (RISCV_IRQ_ASYNC + 5)  /* Supervisor Timer Int */
#define RISCV_IRQ_MTIMER        (RISCV_IRQ_ASYNC + 7)  /* Machine Timer Int */
#define RISCV_IRQ_SEXT          (RISCV_IRQ_ASYNC + 9)  /* Supervisor External Int */
#define RISCV_IRQ_MEXT          (RISCV_IRQ_ASYNC + 11) /* Machine External Int */
#define RISCV_IRQ_HPMOV         (RISCV_IRQ_ASYNC + 17) /* HPM Overflow Int */

/* IRQ bit and IRQ mask */

#ifdef CONFIG_ARCH_RV32
#  define RISCV_IRQ_BIT           (UINT32_C(1) << 31)
#else
#  define RISCV_IRQ_BIT           (UINT64_C(1) << 63)
#endif

#define RISCV_IRQ_MASK            (~RISCV_IRQ_BIT)

/* Configuration ************************************************************/

/* Processor PC */

#define REG_EPC_NDX         0

/* General purpose registers
 * $0: Zero register does not need to be saved
 * $1: ra (return address)
 */

#define REG_X1_NDX          1

/* $2: Stack POinter
 * $3: Global Pointer
 * $4: Thread Pointer
 */

#define REG_X2_NDX          2
#define REG_X3_NDX          3
#define REG_X4_NDX          4

/* $5-$7 = t0-t2: Temporary registers */

#define REG_X5_NDX          5
#define REG_X6_NDX          6
#define REG_X7_NDX          7

/* $8: s0 / fp Frame pointer */

#define REG_X8_NDX          8

/* $9 s1 Saved register */

#define REG_X9_NDX          9

/* $10-$17 = a0-a7: Argument registers */

#define REG_X10_NDX         10
#define REG_X11_NDX         11
#define REG_X12_NDX         12
#define REG_X13_NDX         13
#define REG_X14_NDX         14
#define REG_X15_NDX         15
#define REG_X16_NDX         16
#define REG_X17_NDX         17

/* $18-$27 = s2-s11: Saved registers */

#define REG_X18_NDX         18
#define REG_X19_NDX         19
#define REG_X20_NDX         20
#define REG_X21_NDX         21
#define REG_X22_NDX         22
#define REG_X23_NDX         23
#define REG_X24_NDX         24
#define REG_X25_NDX         25
#define REG_X26_NDX         26
#define REG_X27_NDX         27

/* $28-31 = t3-t6: Temporary (Volatile) registers */

#define REG_X28_NDX         28
#define REG_X29_NDX         29
#define REG_X30_NDX         30
#define REG_X31_NDX         31

/* Interrupt Context register */

#define REG_INT_CTX_NDX     32

#ifdef CONFIG_ARCH_RISCV_INTXCPT_EXTREGS
#  define INT_XCPT_REGS     (33 + CONFIG_ARCH_RISCV_INTXCPT_EXTREGS)
#else
#  define INT_XCPT_REGS     33
#endif

#ifdef CONFIG_ARCH_RV32
#  define INT_REG_SIZE      4
#else
#  define INT_REG_SIZE      8
#endif

#define INT_XCPT_SIZE       (INT_REG_SIZE * INT_XCPT_REGS)

#ifdef CONFIG_ARCH_RV32
#  if defined(CONFIG_ARCH_QPFPU)
#    define FPU_REG_SIZE    4
#  elif defined(CONFIG_ARCH_DPFPU)
#    define FPU_REG_SIZE    2
#  elif defined(CONFIG_ARCH_FPU)
#    define FPU_REG_SIZE    1
#  endif
#else
#  if defined(CONFIG_ARCH_QPFPU)
#    define FPU_REG_SIZE    2
#  else
#    define FPU_REG_SIZE    1
#  endif
#endif

#ifdef CONFIG_ARCH_FPU
#  define REG_F0_NDX        (FPU_REG_SIZE * 0)
#  define REG_F1_NDX        (FPU_REG_SIZE * 1)
#  define REG_F2_NDX        (FPU_REG_SIZE * 2)
#  define REG_F3_NDX        (FPU_REG_SIZE * 3)
#  define REG_F4_NDX        (FPU_REG_SIZE * 4)
#  define REG_F5_NDX        (FPU_REG_SIZE * 5)
#  define REG_F6_NDX        (FPU_REG_SIZE * 6)
#  define REG_F7_NDX        (FPU_REG_SIZE * 7)
#  define REG_F8_NDX        (FPU_REG_SIZE * 8)
#  define REG_F9_NDX        (FPU_REG_SIZE * 9)
#  define REG_F10_NDX       (FPU_REG_SIZE * 10)
#  define REG_F11_NDX       (FPU_REG_SIZE * 11)
#  define REG_F12_NDX       (FPU_REG_SIZE * 12)
#  define REG_F13_NDX       (FPU_REG_SIZE * 13)
#  define REG_F14_NDX       (FPU_REG_SIZE * 14)
#  define REG_F15_NDX       (FPU_REG_SIZE * 15)
#  define REG_F16_NDX       (FPU_REG_SIZE * 16)
#  define REG_F17_NDX       (FPU_REG_SIZE * 17)
#  define REG_F18_NDX       (FPU_REG_SIZE * 18)
#  define REG_F19_NDX       (FPU_REG_SIZE * 19)
#  define REG_F20_NDX       (FPU_REG_SIZE * 20)
#  define REG_F21_NDX       (FPU_REG_SIZE * 21)
#  define REG_F22_NDX       (FPU_REG_SIZE * 22)
#  define REG_F23_NDX       (FPU_REG_SIZE * 23)
#  define REG_F24_NDX       (FPU_REG_SIZE * 24)
#  define REG_F25_NDX       (FPU_REG_SIZE * 25)
#  define REG_F26_NDX       (FPU_REG_SIZE * 26)
#  define REG_F27_NDX       (FPU_REG_SIZE * 27)
#  define REG_F28_NDX       (FPU_REG_SIZE * 28)
#  define REG_F29_NDX       (FPU_REG_SIZE * 29)
#  define REG_F30_NDX       (FPU_REG_SIZE * 30)
#  define REG_F31_NDX       (FPU_REG_SIZE * 31)
#  define REG_FCSR_NDX      (FPU_REG_SIZE * 32)

#  define FPU_XCPT_REGS     (FPU_REG_SIZE * 33)
#  define FPU_XCPT_SIZE     (INT_REG_SIZE * FPU_XCPT_REGS)
#else /* !CONFIG_ARCH_FPU */
#  define FPU_XCPT_REGS     (0)
#  define FPU_XCPT_SIZE     (0)
#endif /* CONFIG_ARCH_FPU */

#define XCPTCONTEXT_REGS    (INT_XCPT_REGS + FPU_XCPT_REGS)

#ifdef CONFIG_ARCH_LAZYFPU
/* Save only integer regs. FPU is handled separately */

#define XCPTCONTEXT_SIZE    (INT_XCPT_SIZE)
#else
/* Save FPU registers with the integer registers */

#define XCPTCONTEXT_SIZE    (INT_XCPT_SIZE + FPU_XCPT_SIZE)
#endif

#ifdef CONFIG_ARCH_RV_ISA_V
#  define REG_VSTART_NDX    (0)
#  define REG_VTYPE_NDX     (1)
#  define REG_VL_NDX        (2)
#  define REG_VCSR_NDX      (3)
#  define REG_VLENB_NDX     (4)

#  define VPU_XCPT_REGS     (5)
#  define VPU_XCPT_SIZE     (INT_REG_SIZE * VPU_XCPT_REGS)

#  if CONFIG_ARCH_RV_VECTOR_BYTE_LENGTH > 0

/* There are 32 vector registers(v0 - v31) with vlenb length. */

#    define VPU_XCPTC_SIZE  (CONFIG_ARCH_RV_VECTOR_BYTE_LENGTH * 32 + VPU_XCPT_SIZE)

#  endif
#else /* !CONFIG_ARCH_RV_ISA_V */
#  define VPU_XCPT_REGS     (0)
#  define VPU_XCPT_SIZE     (0)
#  define VPU_XCPTC_SIZE    (0)
#endif /* CONFIG_ARCH_RV_ISA_V */

/* In assembly language, values have to be referenced as byte address
 * offsets.  But in C, it is more convenient to reference registers as
 * register save table offsets.
 */

#ifdef __ASSEMBLY__
#  define REG_EPC           (INT_REG_SIZE*REG_EPC_NDX)
#  define REG_X1            (INT_REG_SIZE*REG_X1_NDX)
#  define REG_X2            (INT_REG_SIZE*REG_X2_NDX)
#  define REG_X3            (INT_REG_SIZE*REG_X3_NDX)
#  define REG_X4            (INT_REG_SIZE*REG_X4_NDX)
#  define REG_X5            (INT_REG_SIZE*REG_X5_NDX)
#  define REG_X6            (INT_REG_SIZE*REG_X6_NDX)
#  define REG_X7            (INT_REG_SIZE*REG_X7_NDX)
#  define REG_X8            (INT_REG_SIZE*REG_X8_NDX)
#  define REG_X9            (INT_REG_SIZE*REG_X9_NDX)
#  define REG_X10           (INT_REG_SIZE*REG_X10_NDX)
#  define REG_X11           (INT_REG_SIZE*REG_X11_NDX)
#  define REG_X12           (INT_REG_SIZE*REG_X12_NDX)
#  define REG_X13           (INT_REG_SIZE*REG_X13_NDX)
#  define REG_X14           (INT_REG_SIZE*REG_X14_NDX)
#  define REG_X15           (INT_REG_SIZE*REG_X15_NDX)
#  define REG_X16           (INT_REG_SIZE*REG_X16_NDX)
#  define REG_X17           (INT_REG_SIZE*REG_X17_NDX)
#  define REG_X18           (INT_REG_SIZE*REG_X18_NDX)
#  define REG_X19           (INT_REG_SIZE*REG_X19_NDX)
#  define REG_X20           (INT_REG_SIZE*REG_X20_NDX)
#  define REG_X21           (INT_REG_SIZE*REG_X21_NDX)
#  define REG_X22           (INT_REG_SIZE*REG_X22_NDX)
#  define REG_X23           (INT_REG_SIZE*REG_X23_NDX)
#  define REG_X24           (INT_REG_SIZE*REG_X24_NDX)
#  define REG_X25           (INT_REG_SIZE*REG_X25_NDX)
#  define REG_X26           (INT_REG_SIZE*REG_X26_NDX)
#  define REG_X27           (INT_REG_SIZE*REG_X27_NDX)
#  define REG_X28           (INT_REG_SIZE*REG_X28_NDX)
#  define REG_X29           (INT_REG_SIZE*REG_X29_NDX)
#  define REG_X30           (INT_REG_SIZE*REG_X30_NDX)
#  define REG_X31           (INT_REG_SIZE*REG_X31_NDX)
#  define REG_INT_CTX       (INT_REG_SIZE*REG_INT_CTX_NDX)

#ifdef CONFIG_ARCH_FPU
#  define REG_F0            (INT_REG_SIZE*REG_F0_NDX)
#  define REG_F1            (INT_REG_SIZE*REG_F1_NDX)
#  define REG_F2            (INT_REG_SIZE*REG_F2_NDX)
#  define REG_F3            (INT_REG_SIZE*REG_F3_NDX)
#  define REG_F4            (INT_REG_SIZE*REG_F4_NDX)
#  define REG_F5            (INT_REG_SIZE*REG_F5_NDX)
#  define REG_F6            (INT_REG_SIZE*REG_F6_NDX)
#  define REG_F7            (INT_REG_SIZE*REG_F7_NDX)
#  define REG_F8            (INT_REG_SIZE*REG_F8_NDX)
#  define REG_F9            (INT_REG_SIZE*REG_F9_NDX)
#  define REG_F10           (INT_REG_SIZE*REG_F10_NDX)
#  define REG_F11           (INT_REG_SIZE*REG_F11_NDX)
#  define REG_F12           (INT_REG_SIZE*REG_F12_NDX)
#  define REG_F13           (INT_REG_SIZE*REG_F13_NDX)
#  define REG_F14           (INT_REG_SIZE*REG_F14_NDX)
#  define REG_F15           (INT_REG_SIZE*REG_F15_NDX)
#  define REG_F16           (INT_REG_SIZE*REG_F16_NDX)
#  define REG_F17           (INT_REG_SIZE*REG_F17_NDX)
#  define REG_F18           (INT_REG_SIZE*REG_F18_NDX)
#  define REG_F19           (INT_REG_SIZE*REG_F19_NDX)
#  define REG_F20           (INT_REG_SIZE*REG_F20_NDX)
#  define REG_F21           (INT_REG_SIZE*REG_F21_NDX)
#  define REG_F22           (INT_REG_SIZE*REG_F22_NDX)
#  define REG_F23           (INT_REG_SIZE*REG_F23_NDX)
#  define REG_F24           (INT_REG_SIZE*REG_F24_NDX)
#  define REG_F25           (INT_REG_SIZE*REG_F25_NDX)
#  define REG_F26           (INT_REG_SIZE*REG_F26_NDX)
#  define REG_F27           (INT_REG_SIZE*REG_F27_NDX)
#  define REG_F28           (INT_REG_SIZE*REG_F28_NDX)
#  define REG_F29           (INT_REG_SIZE*REG_F29_NDX)
#  define REG_F30           (INT_REG_SIZE*REG_F30_NDX)
#  define REG_F31           (INT_REG_SIZE*REG_F31_NDX)
#  define REG_FCSR          (INT_REG_SIZE*REG_FCSR_NDX)
#endif

#ifdef CONFIG_ARCH_RV_ISA_V
#  define REG_VSTART        (INT_REG_SIZE*REG_VSTART_NDX)
#  define REG_VTYPE         (INT_REG_SIZE*REG_VTYPE_NDX)
#  define REG_VL            (INT_REG_SIZE*REG_VL_NDX)
#  define REG_VCSR          (INT_REG_SIZE*REG_VCSR_NDX)
#  define REG_VLENB         (INT_REG_SIZE*REG_VLENB_NDX)
#endif

#else
#  define REG_EPC           REG_EPC_NDX
#  define REG_X1            REG_X1_NDX
#  define REG_X2            REG_X2_NDX
#  define REG_X3            REG_X3_NDX
#  define REG_X4            REG_X4_NDX
#  define REG_X5            REG_X5_NDX
#  define REG_X6            REG_X6_NDX
#  define REG_X7            REG_X7_NDX
#  define REG_X8            REG_X8_NDX
#  define REG_X9            REG_X9_NDX
#  define REG_X10           REG_X10_NDX
#  define REG_X11           REG_X11_NDX
#  define REG_X12           REG_X12_NDX
#  define REG_X13           REG_X13_NDX
#  define REG_X14           REG_X14_NDX
#  define REG_X15           REG_X15_NDX
#  define REG_X16           REG_X16_NDX
#  define REG_X17           REG_X17_NDX
#  define REG_X18           REG_X18_NDX
#  define REG_X19           REG_X19_NDX
#  define REG_X20           REG_X20_NDX
#  define REG_X21           REG_X21_NDX
#  define REG_X22           REG_X22_NDX
#  define REG_X23           REG_X23_NDX
#  define REG_X24           REG_X24_NDX
#  define REG_X25           REG_X25_NDX
#  define REG_X26           REG_X26_NDX
#  define REG_X27           REG_X27_NDX
#  define REG_X28           REG_X28_NDX
#  define REG_X29           REG_X29_NDX
#  define REG_X30           REG_X30_NDX
#  define REG_X31           REG_X31_NDX
#  define REG_INT_CTX       REG_INT_CTX_NDX

#ifdef CONFIG_ARCH_FPU
#  define REG_F0            REG_F0_NDX
#  define REG_F1            REG_F1_NDX
#  define REG_F2            REG_F2_NDX
#  define REG_F3            REG_F3_NDX
#  define REG_F4            REG_F4_NDX
#  define REG_F5            REG_F5_NDX
#  define REG_F6            REG_F6_NDX
#  define REG_F7            REG_F7_NDX
#  define REG_F8            REG_F8_NDX
#  define REG_F9            REG_F9_NDX
#  define REG_F10           REG_F10_NDX
#  define REG_F11           REG_F11_NDX
#  define REG_F12           REG_F12_NDX
#  define REG_F13           REG_F13_NDX
#  define REG_F14           REG_F14_NDX
#  define REG_F15           REG_F15_NDX
#  define REG_F16           REG_F16_NDX
#  define REG_F17           REG_F17_NDX
#  define REG_F18           REG_F18_NDX
#  define REG_F19           REG_F19_NDX
#  define REG_F20           REG_F20_NDX
#  define REG_F21           REG_F21_NDX
#  define REG_F22           REG_F22_NDX
#  define REG_F23           REG_F23_NDX
#  define REG_F24           REG_F24_NDX
#  define REG_F25           REG_F25_NDX
#  define REG_F26           REG_F26_NDX
#  define REG_F27           REG_F27_NDX
#  define REG_F28           REG_F28_NDX
#  define REG_F29           REG_F29_NDX
#  define REG_F30           REG_F30_NDX
#  define REG_F31           REG_F31_NDX
#  define REG_FCSR          REG_FCSR_NDX
#endif

#ifdef CONFIG_ARCH_RV_ISA_V
#  define REG_VSTART        REG_VSTART_NDX
#  define REG_VTYPE         REG_VTYPE_NDX
#  define REG_VL            REG_VL_NDX
#  define REG_VCSR          REG_VCSR_NDX
#  define REG_VLENB         REG_VLENB_NDX
#endif

#endif

/* Now define more user friendly alternative name that can be used either
 * in assembly or C contexts.
 */

/* $1 = ra: Return address */

#define REG_RA              REG_X1

/* $2 = sp:  The value of the stack pointer on return from the exception */

#define REG_SP              REG_X2

/* $3 = gp: Only needs to be saved under conditions where there are
 * multiple, per-thread values for the GP.
 */

#define REG_GP              REG_X3

/* $4 = tp:  Thread Pointer */

#define REG_TP              REG_X4

/* $5-$7 = t0-t2: Caller saved temporary registers */

#define REG_T0              REG_X5
#define REG_T1              REG_X6
#define REG_T2              REG_X7

/* $8 = either s0 or fp:  Depends if a frame pointer is used or not */

#define REG_S0              REG_X8
#define REG_FP              REG_X8

/* $9 = s1: Caller saved register */

#define REG_S1              REG_X9

/* $10-$17 = a0-a7: Argument registers */

#define REG_A0              REG_X10
#define REG_A1              REG_X11
#define REG_A2              REG_X12
#define REG_A3              REG_X13
#define REG_A4              REG_X14
#define REG_A5              REG_X15
#define REG_A6              REG_X16
#define REG_A7              REG_X17

/* $18-$27 = s2-s11: Callee saved registers */

#define REG_S2              REG_X18
#define REG_S3              REG_X19
#define REG_S4              REG_X20
#define REG_S5              REG_X21
#define REG_S6              REG_X22
#define REG_S7              REG_X23
#define REG_S8              REG_X24
#define REG_S9              REG_X25
#define REG_S10             REG_X26
#define REG_S11             REG_X27

/* $28-$31 = t3-t6: Caller saved temporary registers */

#define REG_T3              REG_X28
#define REG_T4              REG_X29
#define REG_T5              REG_X30
#define REG_T6              REG_X31

#ifdef CONFIG_ARCH_FPU
/* $0-$1 = fs0-fs1: Callee saved registers */

#  define REG_FS0           REG_F8
#  define REG_FS1           REG_F9

/* $18-$27 = fs2-fs11: Callee saved registers */

#  define REG_FS2           REG_F18
#  define REG_FS3           REG_F19
#  define REG_FS4           REG_F20
#  define REG_FS5           REG_F21
#  define REG_FS6           REG_F22
#  define REG_FS7           REG_F23
#  define REG_FS8           REG_F24
#  define REG_FS9           REG_F25
#  define REG_FS10          REG_F26
#  define REG_FS11          REG_F27
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifndef __ASSEMBLY__

/* The following structure is included in the TCB and defines the complete
 * state of the thread.
 */

struct xcptcontext
{
  /* These additional register save locations are used to implement the
   * signal delivery trampoline.
   *
   * REVISIT:  Because there is only a reference of these save areas,
   * only a single signal handler can be active.  This precludes
   * queuing of signal actions.  As a result, signals received while
   * another signal handler is executing will be ignored!
   */

  uintreg_t *saved_regs;

#ifndef CONFIG_BUILD_FLAT
  /* This is the saved address to use when returning from a user-space
   * signal handler.
   */

  uintptr_t sigreturn;
#endif

#ifdef CONFIG_ARCH_ADDRENV
#ifdef CONFIG_ARCH_KERNEL_STACK
  /* In this configuration, all syscalls execute from an internal kernel
   * stack.  Why?  Because when we instantiate and initialize the address
   * environment of the new user process, we will temporarily lose the
   * address environment of the old user process, including its stack
   * contents.  The kernel C logic will crash immediately with no valid
   * stack in place.
   */

  uintptr_t *ustkptr;  /* Saved user stack pointer */
  uintptr_t *kstack;   /* Allocate base of the (aligned) kernel stack */
  uintptr_t *ktopstk;  /* Top of kernel stack */
  uintptr_t *kstkptr;  /* Saved kernel stack pointer */
#endif
#endif

  /* Integer register save area */

  uintreg_t *regs;
#ifndef CONFIG_BUILD_FLAT
  uintreg_t *initregs;
#endif

#ifdef CONFIG_LIB_SYSCALL
  /* User integer registers upon system call entry */

  uintreg_t *sregs;
#endif

  /* FPU register save area */

#if defined(CONFIG_ARCH_FPU) && defined(CONFIG_ARCH_LAZYFPU)
  uintreg_t fregs[FPU_XCPT_REGS];
#endif

#ifdef CONFIG_ARCH_RV_ISA_V
#  if CONFIG_ARCH_RV_VECTOR_BYTE_LENGTH > 0
  /* There are 32 vector registers(v0 - v31) with vlenb length. */

  uintreg_t vregs[VPU_XCPTC_SIZE];
#  else
  uintreg_t *vregs;
#  endif
#endif
};

#endif /* __ASSEMBLY__ */

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifndef __ASSEMBLY__

/****************************************************************************
 * Inline functions
 ****************************************************************************/

/* Return the current value of the stack pointer */

static inline_function uintptr_t up_getsp(void)
{
  register uintptr_t sp;
  __asm__
  (
    "\tadd  %0, x0, x2\n"
    : "=r"(sp)
  );
  return sp;
}

/****************************************************************************
 * Public Data
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/* g_interrupt_context store irq status */

EXTERN volatile bool g_interrupt_context[CONFIG_SMP_NCPUS];

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: up_irq_enable
 *
 * Description:
 *   Return the current interrupt state and enable interrupts
 *
 ****************************************************************************/

irqstate_t up_irq_enable(void);

/****************************************************************************
 * Name: up_cpu_index
 *
 * Description:
 *   Return the real core number regardless CONFIG_SMP setting,
 *   context aware way to query hart id (physical core ID)
 *
 *   The function up_cpu_index is designed to retrieve the hardware thread
 *   ID (hartid) in different execution modes of RISC-V. Its behavior depends
 *   on the configuration and execution mode:
 *
 *   - In machine mode, up_cpu_index reads directly from the CSR mhartid.
 *   - In supervisor mode, the hartid is stored in the percpu structure
 *     during boot because supervisor mode does not have access to CSR
 *     `shartid`. The SBI (Supervisor Binary Interface) provides the hartid
 *     in the a0 register (as per SBI ABI requirements), and it is the
 *     responsibility of the payload OS to store this value internally.
 *     We use the percpu scratch register for this purpose, as it is the only
 *     location that is unique for each CPU and non-volatile.
 *
 *   Note: In flat (machine) mode, you could still read the hartid from CSR
 *   mhartid even if CONFIG_RISCV_PERCPU_SCRATCH is enabled.
 *
 * Returned Value:
 *   Hart id
 *
 ****************************************************************************/

#ifdef CONFIG_ARCH_HAVE_MULTICPU
#ifdef CONFIG_ARCH_USE_S_MODE
int up_cpu_index(void) noinstrument_function;
#else
noinstrument_function static inline int up_cpu_index(void)
{
  return READ_CSR(CSR_MHARTID);
}
#endif /* CONFIG_ARCH_USE_S_MODE */
#endif /* CONFIG_ARCH_HAVE_MULTICPU */

/****************************************************************************
 * Name: up_this_cpu
 *
 * Description:
 *   Return the logical core number. Default implementation is 1:1 mapping,
 *   i.e. physical=logical.
 *
 ****************************************************************************/

#ifdef CONFIG_ARCH_RV_CPUID_MAP
int up_this_cpu(void);
#else
#define up_this_cpu() up_cpu_index()
#endif /* CONFIG_ARCH_RV_CPUID_MAP */

/****************************************************************************
 * Inline Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_irq_save
 *
 * Description:
 *   Disable interrupts and return the previous value of the mstatus register
 *
 ****************************************************************************/

noinstrument_function static inline_function irqstate_t up_irq_save(void)
{
  irqstate_t flags;

  /* Read mstatus & clear machine interrupt enable (MIE) in mstatus */

  __asm__ __volatile__
    (
      "csrrc %0, " __XSTR(CSR_STATUS) ", %1\n"
      : "=r" (flags)
      : "r"(STATUS_IE)
      : "memory"
    );

  /* Return the previous mstatus value so that it can be restored with
   * up_irq_restore().
   */

  return flags;
}

/****************************************************************************
 * Name: up_irq_restore
 *
 * Description:
 *   Restore the value of the mstatus register
 *
 ****************************************************************************/

noinstrument_function static inline_function
void up_irq_restore(irqstate_t flags)
{
  __asm__ __volatile__
    (
      "csrw " __XSTR(CSR_STATUS) ", %0\n"
      : /* no output */
      : "r" (flags)
      : "memory"
    );
}

/****************************************************************************
 * Name: up_set_interrupt_context
 *
 * Description:
 *   Set the interrupt handler context.
 *
 ****************************************************************************/

noinstrument_function
static inline_function void up_set_interrupt_context(bool flag)
{
#ifdef CONFIG_SMP
  g_interrupt_context[up_this_cpu()] = flag;
#else
  g_interrupt_context[0] = flag;
#endif
}

/****************************************************************************
 * Name: up_interrupt_context
 *
 * Description:
 *   Return true is we are currently executing in the interrupt
 *   handler context.
 *
 ****************************************************************************/

noinstrument_function static inline_function bool up_interrupt_context(void)
{
#ifdef CONFIG_SMP
  irqstate_t flags = up_irq_save();
  bool ret = g_interrupt_context[up_this_cpu()];
  up_irq_restore(flags);
  return ret;
#else
  return g_interrupt_context[0];
#endif
}

/****************************************************************************
 * Name: up_getusrpc
 ****************************************************************************/

#define up_getusrpc(regs) \
    (((uintptr_t *)((regs) ? (regs) : running_regs()))[REG_EPC])

/****************************************************************************
 * Name: up_getusrsp
 ****************************************************************************/

#define up_getusrsp(regs) \
    (((uintptr_t*)(regs))[REG_SP])

#undef EXTERN
#if defined(__cplusplus)
}
#endif
#endif /* __ASSEMBLY__ */

#endif /* __ARCH_RISCV_INCLUDE_IRQ_H */
