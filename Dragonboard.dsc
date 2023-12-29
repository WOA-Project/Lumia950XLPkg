#
#  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.
#  Copyright (c) 2018 - 2023, Bingxing Wang. All rights reserved.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = Dragonboard810
  PLATFORM_GUID                  = 89cc88e5-c50a-42cf-b8d7-2ab8924037b9
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/Dragonboard810-$(ARCH)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Lumia950XLPkg/Dragonboard.fdf

  DEFINE SECURE_BOOT_ENABLE           = FALSE
  DEFINE USE_SCREEN_FOR_SERIAL_OUTPUT = 0
  DEFINE MEMORY_4GB                   = 1
  DEFINE DRAGONBOARD                  = 1
  DEFINE NOPSCI                       = 1

[BuildOptions.common]
  GCC:*_*_AARCH64_CC_FLAGS = -DSILICON_PLATFORM=8994 -DANDROID_RELOC_FRAMEBUFFER=1 -DDRAGONBOARD=1 -DNOPSCI=1
  
[PcdsFixedAtBuild.common]
  # Platform-specific
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x00000000         # 0GB Base
  gArmTokenSpaceGuid.PcdSystemMemorySize|0xC0000000         # 3GB Size (actually 4GB?)
  gEmbeddedTokenSpaceGuid.PcdPrePiStackBase|0x00C00000
  gEmbeddedTokenSpaceGuid.PcdPrePiStackSize|0x00040000      # 256K stack
  gArmPlatformTokenSpaceGuid.PcdCoreCount|8
  gArmPlatformTokenSpaceGuid.PcdClusterCount|2
  gLumia950XLPkgTokenSpaceGuid.PcdSmbiosSystemModel|"Dragonboard 810"
  gLumia950XLPkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Qualcomm Snapdragon 810 Processor (8994)"
  gLumia950XLPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"Dragonboard 810"
  gLumia950XLPkgTokenSpaceGuid.PsciCpuSuspendAddress|0x6c03aa8
  
!include Lumia950XLPkg/Library/UartDmSerialPortLib/UartDmSerialConfigBlkBlsp.dsc.inc
!include Lumia950XLPkg/Shared.dsc.inc
