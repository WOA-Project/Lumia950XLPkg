/** @file
  Implementation for PlatformBootManagerLib library class interfaces.

  Copyright (C) 2015-2016, Red Hat, Inc.
  Copyright (c) 2014, ARM Ltd. All rights reserved.
  Copyright (c) 2004 - 2016, Intel Corporation. All rights reserved.
  Copyright (c) 2016, Linaro Ltd. All rights reserved.
  Copyright (c), 2017, Andrei Warkentin <andrey.warkentin@gmail.com>

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/ArmLib.h>
#include <Library/ArmSmcLib.h>

#include <Library/BootAppLib.h>
#include <Library/BootLogoLib.h>
#include <Library/CapsuleLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiLib.h>

#include <Protocol/DevicePath.h>
#include <Protocol/EsrtManagement.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/LoadedImage.h>

#include <Guid/EventGroup.h>
#include <Guid/TtyTerm.h>

#include <Configuration/BootDevices.h>
#include <Device/TouchDevicePath.h>

#include "PlatformBm.h"

VOID EFIAPI EfiBootManagerConnectAllConsoles(VOID);

#define DP_NODE_LEN(Type)                                                      \
  {                                                                            \
    (UINT8)sizeof(Type), (UINT8)(sizeof(Type) >> 8)                            \
  }

/**
  Check if the handle satisfies a particular condition.

  @param[in] Handle      The handle to check.
  @param[in] ReportText  A caller-allocated string passed in for reporting
                         purposes. It must never be NULL.

  @retval TRUE   The condition is satisfied.
  @retval FALSE  Otherwise. This includes the case when the condition could
not be fully evaluated due to an error.
**/
typedef BOOLEAN(EFIAPI *FILTER_FUNCTION)(
    IN EFI_HANDLE Handle, IN CONST CHAR16 *ReportText);

/**
  Process a handle.

  @param[in] Handle      The handle to process.
  @param[in] ReportText  A caller-allocated string passed in for reporting
                         purposes. It must never be NULL.
**/
typedef VOID(EFIAPI *CALLBACK_FUNCTION)(
    IN EFI_HANDLE Handle, IN CONST CHAR16 *ReportText);

/**
  Locate all handles that carry the specified protocol, filter them with a
  callback function, and pass each handle that passes the filter to another
  callback.

  @param[in] ProtocolGuid  The protocol to look for.

  @param[in] Filter        The filter function to pass each handle to. If this
                           parameter is NULL, then all handles are processed.

  @param[in] Process       The callback function to pass each handle to that
                           clears the filter.
**/
STATIC
VOID FilterAndProcess(
    IN EFI_GUID *ProtocolGuid, IN FILTER_FUNCTION Filter OPTIONAL,
    IN CALLBACK_FUNCTION Process)
{
  EFI_STATUS  Status;
  EFI_HANDLE *Handles;
  UINTN       NoHandles;
  UINTN       Idx;

  Status = gBS->LocateHandleBuffer(
      ByProtocol, ProtocolGuid, NULL /* SearchKey */, &NoHandles, &Handles);
  if (EFI_ERROR(Status)) {
    //
    // This is not an error, just an informative condition.
    //
    DEBUG((EFI_D_VERBOSE, "%a: %g: %r\n", __FUNCTION__, ProtocolGuid, Status));
    return;
  }

  ASSERT(NoHandles > 0);
  for (Idx = 0; Idx < NoHandles; ++Idx) {
    CHAR16 *DevicePathText;
    STATIC CHAR16 Fallback[] = L"<device path unavailable>";

    //
    // The ConvertDevicePathToText() function handles NULL input transparently.
    //
    DevicePathText = ConvertDevicePathToText(
        DevicePathFromHandle(Handles[Idx]),
        FALSE, // DisplayOnly
        FALSE  // AllowShortcuts
    );
    if (DevicePathText == NULL) {
      DevicePathText = Fallback;
    }

    if (Filter == NULL || Filter(Handles[Idx], DevicePathText)) {
      Process(Handles[Idx], DevicePathText);
    }

    if (DevicePathText != Fallback) {
      FreePool(DevicePathText);
    }
  }
  gBS->FreePool(Handles);
}

/**
  This CALLBACK_FUNCTION retrieves the EFI_DEVICE_PATH_PROTOCOL from the
  handle, and adds it to ConOut and ErrOut.
**/
STATIC
VOID EFIAPI AddOutput(IN EFI_HANDLE Handle, IN CONST CHAR16 *ReportText)
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL *DevicePath;

  DevicePath = DevicePathFromHandle(Handle);
  if (DevicePath == NULL) {
    DEBUG(
        (EFI_D_ERROR, "%a: %s: handle %p: device path not found\n",
         __FUNCTION__, ReportText, Handle));
    return;
  }

  Status = EfiBootManagerUpdateConsoleVariable(ConOut, DevicePath, NULL);
  if (EFI_ERROR(Status)) {
    DEBUG(
        (EFI_D_INFO, "%a: %s: adding to ConOut: %r\n", __FUNCTION__, ReportText,
         Status));
    return;
  }

  Status = EfiBootManagerUpdateConsoleVariable(ErrOut, DevicePath, NULL);
  if (EFI_ERROR(Status)) {
    DEBUG(
        (EFI_D_INFO, "%a: %s: adding to ErrOut: %r\n", __FUNCTION__, ReportText,
         Status));
    return;
  }

  DEBUG(
      (EFI_D_VERBOSE, "%a: %s: added to ConOut and ErrOut\n", __FUNCTION__,
       ReportText));
}

STATIC
VOID EFIAPI AddInput(IN EFI_HANDLE Handle, IN CONST CHAR16 *ReportText)
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL *DevicePath;

  DevicePath = DevicePathFromHandle(Handle);
  if (DevicePath == NULL) {
    DEBUG(
        (EFI_D_ERROR, "%a: %s: handle %p: device path not found\n",
         __FUNCTION__, ReportText, Handle));
    return;
  }

  Status = EfiBootManagerUpdateConsoleVariable(ConIn, DevicePath, NULL);
  if (EFI_ERROR(Status)) {
    DEBUG(
        (EFI_D_INFO, "%a: %s: adding to ConIn: %r\n", __FUNCTION__, ReportText,
         Status));
    return;
  }

  DEBUG(
      (EFI_D_VERBOSE, "%a: %s: added to ConOut and ErrOut\n", __FUNCTION__,
       ReportText));
}

STATIC
INTN PlatformRegisterBootOption(
    EFI_DEVICE_PATH_PROTOCOL *DevicePath, CHAR16 *Description,
    UINT32 Attributes)
{
  EFI_STATUS                    Status;
  INTN                          OptionIndex;
  EFI_BOOT_MANAGER_LOAD_OPTION  NewOption;
  EFI_BOOT_MANAGER_LOAD_OPTION *BootOptions;
  UINTN                         BootOptionCount;

  Status = EfiBootManagerInitializeLoadOption(
      &NewOption, LoadOptionNumberUnassigned, LoadOptionTypeBoot, Attributes,
      Description, DevicePath, NULL, 0);
  ASSERT_EFI_ERROR(Status);

  BootOptions =
      EfiBootManagerGetLoadOptions(&BootOptionCount, LoadOptionTypeBoot);

  OptionIndex =
      EfiBootManagerFindLoadOption(&NewOption, BootOptions, BootOptionCount);

  if (OptionIndex == -1) {
    Status = EfiBootManagerAddLoadOptionVariable(&NewOption, MAX_UINTN);
    ASSERT_EFI_ERROR(Status);
    OptionIndex = BootOptionCount;
  }

  EfiBootManagerFreeLoadOption(&NewOption);
  EfiBootManagerFreeLoadOptions(BootOptions, BootOptionCount);

  return OptionIndex;
}

STATIC
INTN PlatformRegisterFvBootOption(
    CONST EFI_GUID *FileGuid, CHAR16 *Description, UINT32 Attributes)
{
  EFI_STATUS                        Status;
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH FileNode;
  EFI_LOADED_IMAGE_PROTOCOL *       LoadedImage;
  EFI_DEVICE_PATH_PROTOCOL *        DevicePath;
  INTN                              OptionIndex;

  Status = gBS->HandleProtocol(
      gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage);
  ASSERT_EFI_ERROR(Status);

  EfiInitializeFwVolDevicepathNode(&FileNode, FileGuid);
  DevicePath = DevicePathFromHandle(LoadedImage->DeviceHandle);
  ASSERT(DevicePath != NULL);
  DevicePath =
      AppendDevicePathNode(DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&FileNode);
  ASSERT(DevicePath != NULL);

  OptionIndex = PlatformRegisterBootOption(DevicePath, Description, Attributes);
  FreePool(DevicePath);

  return OptionIndex;
}

STATIC
VOID PlatformRegisterOptionsAndKeys(VOID)
{
  INTN          ShellOption;
  INTN          SplashOption;
  EFI_INPUT_KEY VolUpBtn;
  EFI_STATUS    Status;

  ShellOption = PlatformRegisterFvBootOption(
      &gUefiShellFileGuid, L"UEFI Shell", LOAD_OPTION_ACTIVE);

  if (ShellOption != -1) {
    VolUpBtn.ScanCode    = SCAN_UP;
    VolUpBtn.UnicodeChar = SCAN_UP;
    Status               = EfiBootManagerAddKeyOptionVariable(
        NULL, ShellOption, 0, &VolUpBtn, NULL);
    ASSERT(Status == EFI_SUCCESS || Status == EFI_ALREADY_STARTED);
  }

  SplashOption = PlatformRegisterFvBootOption(
      &gLumiaBootSplashAppGuid, LUMIA_BOOTAPP_TITLE, LOAD_OPTION_ACTIVE);
  ASSERT(SplashOption != -1);
}

STATIC
VOID PlatformRegisterSetupKey(VOID)
{
  EFI_STATUS                   Status;
  EFI_INPUT_KEY                PowerBtn;
  EFI_INPUT_KEY                EscBtn;
  EFI_BOOT_MANAGER_LOAD_OPTION BootOption;

  //
  // Map Power to Boot Manager Menu
  //
  PowerBtn.ScanCode    = SCAN_NULL;
  PowerBtn.UnicodeChar = CHAR_CARRIAGE_RETURN;
  EscBtn.ScanCode      = SCAN_ESC;
  EscBtn.UnicodeChar   = CHAR_NULL;
  Status               = EfiBootManagerGetBootManagerMenu(&BootOption);
  ASSERT_EFI_ERROR(Status);
  Status = EfiBootManagerAddKeyOptionVariable(
      NULL, (UINT16)BootOption.OptionNumber, 0, &PowerBtn, NULL);
  ASSERT(Status == EFI_SUCCESS || Status == EFI_ALREADY_STARTED);
  Status = EfiBootManagerAddKeyOptionVariable(
      NULL, (UINT16)BootOption.OptionNumber, 0, &EscBtn, NULL);
  ASSERT(Status == EFI_SUCCESS || Status == EFI_ALREADY_STARTED);
}

//
// BDS Platform Functions
//
/**
  Do the platform init, can be customized by OEM/IBV
  Possible things that can be done in PlatformBootManagerBeforeConsole:
  > Update console variable: 1. include hot-plug devices;
  >                          2. Clear ConIn and add SOL for AMT
  > Register new Driver#### or Boot####
  > Register new Key####: e.g.: F12
  > Signal ReadyToLock event
  > Authentication action: 1. connect Auth devices;
  >                        2. Identify auto logon user.
**/
VOID EFIAPI PlatformBootManagerBeforeConsole(VOID)
{
  EFI_STATUS                Status;
  ESRT_MANAGEMENT_PROTOCOL *EsrtManagement;

  if (GetBootModeHob() == BOOT_ON_FLASH_UPDATE) {
    DEBUG((DEBUG_INFO, "ProcessCapsules Before EndOfDxe ......\n"));
    Status = ProcessCapsules();
    DEBUG((DEBUG_INFO, "ProcessCapsules returned %r\n", Status));
  }
  else {
    Status = gBS->LocateProtocol(
        &gEsrtManagementProtocolGuid, NULL, (VOID **)&EsrtManagement);
    if (!EFI_ERROR(Status)) {
      EsrtManagement->SyncEsrtFmp();
    }
  }

  //
  // Signal EndOfDxe PI Event
  //
  EfiEventGroupSignal(&gEfiEndOfDxeEventGroupGuid);

  //
  // Dispatch deferred images after EndOfDxe event and ReadyToLock installation.
  //
  EfiBootManagerDispatchDeferredImages();

  //
  // Now add the device path of all handles with GOP on them to ConOut and
  // ErrOut.
  //
  FilterAndProcess(&gEfiGraphicsOutputProtocolGuid, NULL, AddOutput);

  //
  // Now add the device path of all handles with QcomKeypadDeviceProtocolGuid
  // on them to ConIn.
  //
  FilterAndProcess(&gEFIDroidKeypadDeviceProtocolGuid, NULL, AddInput);

  // Register setup key then
  PlatformRegisterSetupKey();
  PlatformRegisterOptionsAndKeys();
}

/**
  Do the platform specific action after the console is ready
  Possible things that can be done in PlatformBootManagerAfterConsole:
  > Console post action:
    > Dynamically switch output mode from 100x31 to 80x25 for certain scenarios
    > Signal console ready platform customized event
  > Run diagnostics like memory testing
  > Connect certain devices
  > Dispatch aditional option roms
  > Special boot: e.g.: USB boot, enter UI
**/
VOID EFIAPI PlatformBootManagerAfterConsole(VOID)
{
  ESRT_MANAGEMENT_PROTOCOL *    EsrtManagement;
  EFI_BOOT_MANAGER_LOAD_OPTION *BootOptions;
  UINTN                         BootOptionCount;
  EFI_STATUS                    Status;

  //
  // Show the splash screen.
  //
  Status = BootLogoEnableLogo();

  //
  // Show version information.
  //
  Print(L"Lumia AArch64 Bootstrap, version %a \n", __IMPL_COMMIT_ID__);
  Print(L"EDK2 base %a \n", __EDK2_RELEASE__);
  Print(L"Built by %a on %a \n", __BUILD_OWNER__, __RELEASE_DATE__);
#ifdef CLANG
  Print(L"Built using Clang %a\n", __clang_version__);
#endif

  //
  // Connect the rest of the devices.
  //
  EfiBootManagerConnectAll();
  EfiBootManagerConnectAllConsoles();

  Status = gBS->LocateProtocol(
      &gEsrtManagementProtocolGuid, NULL, (VOID **)&EsrtManagement);
  if (!EFI_ERROR(Status)) {
    EsrtManagement->SyncEsrtFmp();
  }

  if (GetBootModeHob() == BOOT_ON_FLASH_UPDATE) {
    DEBUG((DEBUG_INFO, "ProcessCapsules After EndOfDxe ......\n"));
    Status = ProcessCapsules();
    DEBUG((DEBUG_INFO, "ProcessCapsules returned %r\n", Status));
  }

  EfiBootManagerRefreshAllBootOption();

  BootOptions =
      EfiBootManagerGetLoadOptions(&BootOptionCount, LoadOptionTypeBoot);
  ASSERT(BootOptionCount != -1);
  for (UINTN i = 0; i < BootOptionCount; i++) {
    if (StrCmp(LUMIA_BOOTAPP_TITLE, BootOptions[i].Description) == 0) {
      EfiBootManagerBoot(&BootOptions[i]);
    }
  }

  EfiBootManagerFreeLoadOptions(BootOptions, BootOptionCount);
}

/**
  This function is called each second during the boot manager waits the
  timeout.

  @param TimeoutRemain  The remaining timeout.
**/
VOID EFIAPI PlatformBootManagerWaitCallback(UINT16 TimeoutRemain)
{
  // Not using now
}

/**
  The function is called when no boot option could be launched,
  including platform recovery options and options pointing to applications
  built into firmware volumes.
  If this function returns, BDS attempts to enter an infinite loop.
**/
VOID EFIAPI PlatformBootManagerUnableToBoot(VOID)
{
  EFI_STATUS                   Status;
  EFI_BOOT_MANAGER_LOAD_OPTION BootManagerMenu;

  //
  // BootManagerMenu doesn't contain the correct information when return status
  // is EFI_NOT_FOUND.
  //
  Status = EfiBootManagerGetBootManagerMenu(&BootManagerMenu);
  if (EFI_ERROR(Status)) {
    return;
  }

  for (;;) {
    EfiBootManagerBoot(&BootManagerMenu);
  }
}
