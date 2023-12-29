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
  PLATFORM_NAME                  = OnePlus2
  PLATFORM_GUID                  = b6325ac2-9f3f-4b1d-b129-ac7b35ddde60
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/OnePlus2-$(ARCH)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Lumia950XLPkg/OnePlus2.fdf

  DEFINE SECURE_BOOT_ENABLE           = TRUE
  DEFINE USE_SCREEN_FOR_SERIAL_OUTPUT = 0
  DEFINE MEMORY_4GB                   = 1
  DEFINE ONEPLUS_2                    = 1
  DEFINE ANDROID_RELOC_FRAMEBUFFER    = 1
  DEFINE ANDROID_MEMMAP               = 1

[BuildOptions.common]
  GCC:*_*_AARCH64_CC_FLAGS = -DSILICON_PLATFORM=8994 -DANDROID_RELOC_FRAMEBUFFER=1 -DONEPLUS2=1 -DANDROID_MEMMAP=1
  
[PcdsFixedAtBuild.common]
  # Platform-specific
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x00000000         # 0GB Base
  gArmTokenSpaceGuid.PcdSystemMemorySize|0xC0000000         # 3GB Size
  gEmbeddedTokenSpaceGuid.PcdPrePiStackBase|0x00C00000
  gEmbeddedTokenSpaceGuid.PcdPrePiStackSize|0x00040000      # 256K stack
  gArmPlatformTokenSpaceGuid.PcdCoreCount|8
  gArmPlatformTokenSpaceGuid.PcdClusterCount|2
  gLumia950XLPkgTokenSpaceGuid.PcdSmbiosSystemModel|"OnePlus 2"
  gLumia950XLPkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Qualcomm Snapdragon 810 Processor (8994)"
  gLumia950XLPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"OnePlus 2"
  gLumia950XLPkgTokenSpaceGuid.SynapticsXMax|1440
  gLumia950XLPkgTokenSpaceGuid.SynapticsYMax|2660
  gLumia950XLPkgTokenSpaceGuid.PsciCpuSuspendAddress|0x6c03aa8
  gLumia950XLPkgTokenSpaceGuid.PcdMipiFrameBufferAddress|0x03400000
  
!include Lumia950XLPkg/Shared.dsc.inc