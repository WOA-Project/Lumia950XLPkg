/** @file
  Support ResetSystem Runtime call using Qualcomm PMIC via SPMI.

  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
  Copyright (c) 2013-2015, ARM Ltd. All rights reserved.<BR>
  Copyright (c) 2014, Linaro Ltd. All rights reserved.<BR>
  Copyright (c) 2018, Bingxing Wang. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
License which accompanies this distribution.  The full text of the license may
be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>

#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/EfiResetSystemLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Library/IoLib.h>
#include <Library/LKEnvLib.h>
#include <Library/dload_util.h>
// Must come in order
#include <Library/QcomPm8x41Lib.h>
#include <Library/RuntimeSpmiLib.h>

#include <Chipset/reboot.h>
#include <Platform/iomap.h>

EFI_EVENT mPmicShutdownVirtualAddressChangedEvent;

STATIC UINTN pPonPsHoldAddressVirtual;
STATIC UINTN pResetReasonAddressVirtual;

/* bit 7: S2_RESET_EN, bit 0:3 : RESET_TYPE  */
#define PON_PS_HOLD_RESET_CTL 0x85A
#define PON_PS_HOLD_RESET_CTL2 0x85B

/* PON Peripheral register bit values */
#define RESIN_ON_INT_BIT 1
#define KPDPWR_ON_INT_BIT 0
#define RESIN_BARK_INT_BIT 4
#define S2_RESET_EN_BIT 7

#define REG_OFFSET(_addr) ((_addr)&0xFF)
#define PERIPH_ID(_addr) (((_addr)&0xFF00) >> 8)
#define SLAVE_ID(_addr) ((_addr) >> 16)

/* SPMI helper function which takes slave id as the i/p */
STATIC VOID rt_pm8xxx_reg_write(uint8_t slave_id, uint32_t addr, uint8_t val)
{
  RUNTIME_PMIC_ARB_CMD   cmd;
  RUNTIME_PMIC_ARB_PARAM param;

  cmd.address  = PERIPH_ID(addr);
  cmd.offset   = REG_OFFSET(addr);
  cmd.slave_id = slave_id;

  cmd.priority = 0;

  param.buffer = &val;
  param.size   = 1;

  RtSpmiPmicArbWriteCmd(&cmd, &param);
}

STATIC VOID rt_pm8994_reset_configure(uint8_t reset_type)
{
  /* Slave ID of pm8994 and pmi8994 */
  uint8_t slave_id[] = {0, 2};
  uint8_t i;

  /* Reset sequence
  1. Disable the ps hold for pm8994
  2. set reset type for both pm8994 & pmi8994
  3. Enable ps hold for pm8994 to trigger the reset
  */
  /* disable PS_HOLD_RESET */
  rt_pm8xxx_reg_write(slave_id[0], PON_PS_HOLD_RESET_CTL2, 0x0);

  /* Delay needed for disable to kick in. */
  udelay(300);

  /* configure reset type */
  for (i = 0; i < ARRAY_SIZE(slave_id); i++)
    rt_pm8xxx_reg_write(slave_id[i], PON_PS_HOLD_RESET_CTL, reset_type);

  /* enable PS_HOLD_RESET */
  rt_pm8xxx_reg_write(
      slave_id[0], PON_PS_HOLD_RESET_CTL2, BIT(S2_RESET_EN_BIT));
}

STATIC VOID shutdown_device(VOID)
{
  /* Configure PMIC for shutdown. */
  rt_pm8994_reset_configure(PON_PSHOLD_SHUTDOWN);

  /* Drop PS_HOLD for MSM */
  writel_rt(0x00, pPonPsHoldAddressVirtual);
  mdelay(5000);

  DEBUG((EFI_D_ERROR, "Shutdown failed\n"));
  ASSERT(0);
}

STATIC VOID reboot_device(UINTN warm)
{
  UINT8 reset_type = 0;

  /* Write the reboot reason */
  writel_rt(0, pResetReasonAddressVirtual);

  if (warm == 1) {
    reset_type = PON_PSHOLD_WARM_RESET;
  }
  else {
    reset_type = PON_PSHOLD_HARD_RESET;
  }

  rt_pm8994_reset_configure(reset_type);

  /* Drop PS_HOLD for MSM */
  writel_rt(0x00, pPonPsHoldAddressVirtual);
  mdelay(5000);

  DEBUG((EFI_D_ERROR, "Rebooting failed\n"));
}

/**
  Resets the entire platform.

  @param  ResetType             The type of reset to perform.
  @param  ResetStatus           The status code for the reset.
  @param  DataSize              The size, in bytes, of WatchdogData.
  @param  ResetData             For a ResetType of EfiResetCold, EfiResetWarm,
or EfiResetShutdown the data buffer starts with a Null-terminated Unicode
string, optionally followed by additional binary data.

**/
EFI_STATUS
EFIAPI
LibResetSystem(
    IN EFI_RESET_TYPE ResetType, IN EFI_STATUS ResetStatus, IN UINTN DataSize,
    IN CHAR16 *ResetData OPTIONAL)
{
  switch (ResetType) {
  case EfiResetPlatformSpecific:
    // Map the platform specific reset as reboot
  case EfiResetWarm:
    // Issue warm reset
    reboot_device(1);
    break;
  case EfiResetCold:
    // Issue cold reset
    reboot_device(0);
    break;
  case EfiResetShutdown:
    shutdown_device();
    break;
  default:
    ASSERT(FALSE);
    return EFI_UNSUPPORTED;
  }

  // We should never be here
  DEBUG((EFI_D_ERROR, "%a: PMIC Reset failed\n", __FUNCTION__));
  CpuDeadLoop();
  return EFI_DEVICE_ERROR;
}

VOID EFIAPI
     LibRuntimeVirtualAddressChanged(IN EFI_EVENT Event, IN VOID *Context)
{
  gRT->ConvertPointer(0, (VOID **)&pPonPsHoldAddressVirtual);

  gRT->ConvertPointer(0, (VOID **)&pResetReasonAddressVirtual);
}

/**
  Initialize any infrastructure required for LibResetSystem () to function.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
LibInitializeResetSystem(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  // Initialize runtime SPMI function
  Status = RtSpmiInitialize();

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Fail to initialize runtime SPMI \n"));
    CpuDeadLoop();
  }

  // Set Address
  pPonPsHoldAddressVirtual   = MPM2_MPM_PS_HOLD;

#if SILICON_PLATFORM == 8994
  pResetReasonAddressVirtual = RESTART_REASON_ADDR;
#elif SILICON_PLATFORM == 8992
  pResetReasonAddressVirtual = RESTART_REASON_ADDR2;
#else
#error "Not a valid MSM platform"
#endif

  // Register Virtual Address Change event.
  Status = gBS->CreateEventEx(
      EVT_NOTIFY_SIGNAL, TPL_NOTIFY, LibRuntimeVirtualAddressChanged, NULL,
      &gEfiEventVirtualAddressChangeGuid,
      &mPmicShutdownVirtualAddressChangedEvent);

  // Do not care
  return Status;
}
