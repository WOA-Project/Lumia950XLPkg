/**@file Console Message Library

Copyright (C) Microsoft Corporation. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/UefiLib.h>
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Library/MuUefiVersionLib.h>

/**
Display the platform specific debug messages
**/
VOID EFIAPI ConsoleMsgLibDisplaySystemInfoOnConsole(VOID)
{
  EFI_STATUS             Status;
  CHAR8                 *uefiDate             = NULL;
  CHAR8                 *uefiVersion          = NULL;
  UINTN                  DateBufferLength     = 0;
  UINTN                  VersionBufferLength  = 0;

  Print(L"Firmware information:\n");

  Status = GetBuildDateStringAscii(NULL, &DateBufferLength);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    uefiDate = (CHAR8 *)AllocateZeroPool(DateBufferLength);
    if (uefiDate != NULL) {
      Status = GetBuildDateStringAscii(uefiDate, &DateBufferLength);
      if (Status == EFI_SUCCESS) {
        Print(L"  UEFI build date: %a\n", uefiDate);
      }
      FreePool(uefiDate);
    }
  }

  Status = GetUefiVersionStringAscii(NULL, &VersionBufferLength);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    uefiVersion = (CHAR8 *)AllocateZeroPool(VersionBufferLength);
    if (uefiVersion != NULL) {
      Status = GetUefiVersionStringAscii(uefiVersion, &VersionBufferLength);
      if (Status == EFI_SUCCESS) {
        Print(L"  UEFI version:    %a\n", uefiVersion);
      }
      FreePool(uefiVersion);
    }
  }

  Print(L"  UEFI flavor:     Plutonium\n");
}
