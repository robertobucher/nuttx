/*********************************************************************************
 * libs/libc/machine/x86_64/cache.h
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2014, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *     * this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright notice,
 *     * this list of conditions and the following disclaimer in the documentation
 *     * and/or other materials provided with the distribution.
 *
 *     * Neither the name of Intel Corporation nor the names of its contributors
 *     * may be used to endorse or promote products derived from this software
 *     * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#ifndef __LIBS_LIBC_MACHINE_X86_64_GNU_CACHE_H
#define __LIBS_LIBC_MACHINE_X86_64_GNU_CACHE_H

/*********************************************************************************
 * Pre-processor Definitions
 *********************************************************************************/

/* Values are optimized for Core Architecture */

#define SHARED_CACHE_SIZE (4096 * 1024)  /* Core Architecture L2 Cache */
#define DATA_CACHE_SIZE   (24 * 1024)    /* Core Architecture L1 Data Cache */

#define SHARED_CACHE_SIZE_HALF (SHARED_CACHE_SIZE / 2)
#define DATA_CACHE_SIZE_HALF   (DATA_CACHE_SIZE / 2)

#endif  /* __LIBS_LIBC_MACHINE_X86_64_GNU_CACHE_H */
