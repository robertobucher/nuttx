############################################################################
# boards/arm/sam34/sam4s-xplained-pro/src/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(TOPDIR)/Make.defs

CSRCS = sam_boot.c

ifeq ($(CONFIG_MMCSD_SPI),y)
CSRCS += sam_spi.c
endif

ifeq ($(CONFIG_BOARDCTL),y)
CSRCS += sam_appinit.c
endif

ifeq ($(CONFIG_SAM34_EXTNAND),y)
ifeq ($(CONFIG_MTD_NAND),y)
CSRCS += sam_nandflash.c
endif
endif

ifeq ($(CONFIG_SAM34_HSMCI),y)
CSRCS += sam_hsmci.c
endif

ifeq ($(CONFIG_SAM34_UDP),y)
CSRCS += sam_udp.c
endif

ifeq ($(CONFIG_ARCH_LEDS),y)
CSRCS += sam_autoleds.c
else
CSRCS += sam_userleds.c
endif

ifeq ($(CONFIG_ARCH_BUTTONS),y)
CSRCS += sam_buttons.c
endif

ifeq ($(CONFIG_SAM34_WDT),y)
CSRCS += sam_wdt.c
endif

include $(TOPDIR)/boards/Board.mk
