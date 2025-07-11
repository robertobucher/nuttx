/****************************************************************************
 * libs/libc/machine/arm/arch_atexit.c
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

#include <nuttx/compiler.h>

#include <nuttx/atexit.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: __aeabi_atexit
 *
 * Description:
 *   Registers static object destructors.  Normally atexit(f) should call
 *   __aeabi_atexit (NULL, f, NULL).  But in the usage model here, static
 *   constructors are initialized at power up and are never destroyed
 *   because they have global scope and must persist for as long as the
 *   embedded device is powered on.
 *
 ****************************************************************************/

int __aeabi_atexit(void *object, void (*func)(void *), void *dso_handle)
{
  return atexit_register(ATTYPE_CXA, (void (*)(void))func, object,
                         dso_handle);
}
