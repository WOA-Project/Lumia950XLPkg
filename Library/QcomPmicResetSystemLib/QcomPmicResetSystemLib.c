/** @file
  Support ResetSystem Runtime call using PSCI calls

  Note: A similar library is implemented in
  ArmVirtPkg/Library/ArmVirtualizationPsciResetSystemLib
  So similar issues might exist in this implementation too.

  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
  Copyright (c) 2013-2015, ARM Ltd. All rights reserved.<BR>
  Copyright (c) 2014, Linaro Ltd. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/EfiResetSystemLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Library/LKEnvLib.h>
#include <Library/IoLib.h>
#include <Library/dload_util.h>
#include <Protocol/QcomPm8x41.h>
#include <Platform/iomap.h>
#include <Chipset/reboot.h>

STATIC QCOM_PM8X41_PROTOCOL *mPm8x41 = NULL;

STATIC VOID shutdown_device(VOID)
{
  /* Configure PMIC for shutdown. */
  mPm8x41->pm8994_reset_configure(PON_PSHOLD_SHUTDOWN);

  /* Drop PS_HOLD for MSM */
  writel(0x00, MPM2_MPM_PS_HOLD);

  mdelay(5000);

  DEBUG((EFI_D_ERROR, "Shutdown failed\n"));

  ASSERT(0);
}

STATIC VOID reboot_device(UINTN reboot_reason)
{
  UINT8 reset_type = 0;
  UINT32 restart_reason_addr;

  restart_reason_addr = RESTART_REASON_ADDR;

  /* Write the reboot reason */
  writel(reboot_reason, restart_reason_addr);

  if((reboot_reason == FASTBOOT_MODE) || (reboot_reason == DLOAD) || (reboot_reason == RECOVERY_MODE))
    reset_type = PON_PSHOLD_WARM_RESET;
  else
    reset_type = PON_PSHOLD_HARD_RESET;

  mPm8x41->pm8994_reset_configure(reset_type);

  /* Drop PS_HOLD for MSM */
  writel(0x00, MPM2_MPM_PS_HOLD);

  mdelay(5000);

  DEBUG((EFI_D_ERROR, "Rebooting failed\n"));
}

/**
  Resets the entire platform.

  @param  ResetType             The type of reset to perform.
  @param  ResetStatus           The status code for the reset.
  @param  DataSize              The size, in bytes, of WatchdogData.
  @param  ResetData             For a ResetType of EfiResetCold, EfiResetWarm, or
                                EfiResetShutdown the data buffer starts with a Null-terminated
                                Unicode string, optionally followed by additional binary data.

**/
EFI_STATUS
EFIAPI
LibResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData OPTIONAL
  )
{
  EFI_STATUS Status;

  if (mPm8x41 == NULL)
  {
    Status = gBS->LocateProtocol(
      &gQcomPm8x41ProtocolGuid, 
      NULL, 
      (VOID **) &mPm8x41);
    if (EFI_ERROR(Status)) return Status;
  }

  switch (ResetType) {
  case EfiResetPlatformSpecific:
    // Map the platform specific reset as reboot
  case EfiResetWarm:
    // Issue cold reset
  case EfiResetCold:
    // Issue cold reset
    reboot_device(0);
    break;
  case EfiResetShutdown:
    shutdown_device();
    break;
  default:
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  // We should never be here
  DEBUG ((EFI_D_ERROR, "%a: PMIC Reset failed\n", __FUNCTION__));
  CpuDeadLoop ();
  return EFI_UNSUPPORTED;
}

/**
  Initialize any infrastructure required for LibResetSystem () to function.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
LibInitializeResetSystem (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}
