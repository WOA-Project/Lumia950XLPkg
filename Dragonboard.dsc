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
  DEFINE ANDROID_MEMMAP               = 1
  DEFINE OMIT_DEBUG_OUTPUT            = 0

[BuildOptions.common]
  GCC:*_*_AARCH64_CC_FLAGS = -DSILICON_PLATFORM=8994 -DMEMORY_4GB=1 -DDRAGONBOARD=1 -DNOPSCI=1 -DANDROID_MEMMAP=1
  
[PcdsFixedAtBuild.common]
  # Platform-specific
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x00000000         # 0GB Base
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x100000000        # 4GB Size
  gEmbeddedTokenSpaceGuid.PcdPrePiStackBase|0x00C00000
  gEmbeddedTokenSpaceGuid.PcdPrePiStackSize|0x00040000      # 256K stack
  gArmPlatformTokenSpaceGuid.PcdCoreCount|8
  gArmPlatformTokenSpaceGuid.PcdClusterCount|2
  gLumia950XLPkgTokenSpaceGuid.PcdSmbiosSystemModel|"Dragonboard 810"
  gLumia950XLPkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Qualcomm Snapdragon 810 Processor (8994)"
  gLumia950XLPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"Dragonboard 810"
  gLumia950XLPkgTokenSpaceGuid.PsciCpuSuspendAddress|0x6c03aa8
  gLumia950XLPkgTokenSpaceGuid.PcdIsLkBuild|TRUE

  # PCIe (RP 0)
  gArmTokenSpaceGuid.PcdPciBusMin|0
  gArmTokenSpaceGuid.PcdPciBusMax|1
  gArmTokenSpaceGuid.PcdPciIoBase|0
  gArmTokenSpaceGuid.PcdPciIoSize|0
  gArmTokenSpaceGuid.PcdPciMmio32Base|0xFF000000
  gArmTokenSpaceGuid.PcdPciMmio32Size|0x00800000
  gArmTokenSpaceGuid.PcdPciMmio64Base|0xFFFFFFFFFFFFFFFF
  gArmTokenSpaceGuid.PcdPciMmio64Size|0

[PcdsFeatureFlag.common]
  gQcomTokenSpaceGuid.PcdInstallRpmProtocol|TRUE
  
!include Lumia950XLPkg/Library/UartDmSerialPortLib/UartDmSerialConfigBlkBlsp.dsc.inc
!include Lumia950XLPkg/Shared.dsc.inc

[LibraryClasses.common]
  # PCIe things are currently only expose to Dragonboard
  PciLib|MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  PciHostBridgeLib|Lumia950XLPkg/Library/QcomPciHostBridgeLib/QcomPciHostBridgeLib.inf
  PciSegmentLib|MdePkg/Library/BasePciSegmentLibPci/BasePciSegmentLibPci.inf
  PciCapLib|OvmfPkg/Library/BasePciCapLib/BasePciCapLib.inf
  PciCapPciSegmentLib|OvmfPkg/Library/BasePciCapPciSegmentLib/BasePciCapPciSegmentLib.inf

[Components.common]
  Lumia950XLPkg/Application/FastbootResetApp/FastbootResetApp.inf

  # Debug only, not ship to FDF
  Lumia950XLPkg/Driver/Usb3PhyDxe/Usb3PhyDxe.inf

  # PCIe things
  ArmPkg/Drivers/ArmPciCpuIo2Dxe/ArmPciCpuIo2Dxe.inf
  Lumia950XLPkg/Driver/PciHostBridgeDxe/PciHostBridgeDxe.inf
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf

  # PCIe periph
  MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpressDxe.inf
  MdeModulePkg/Bus/Pci/SataControllerDxe/SataControllerDxe.inf
  MdeModulePkg/Bus/Pci/EhciDxe/EhciDxe.inf
  MdeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf
