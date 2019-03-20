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

#define DP_NODE_LEN(Type)                                                      \
  {                                                                            \
    (UINT8)sizeof(Type), (UINT8)(sizeof(Type) >> 8)                            \
  }

#pragma pack(1)
typedef struct {
  VENDOR_DEVICE_PATH         SerialDxe;
  UART_DEVICE_PATH           Uart;
  VENDOR_DEFINED_DEVICE_PATH TermType;
  EFI_DEVICE_PATH_PROTOCOL   End;
} PLATFORM_SERIAL_CONSOLE;
#pragma pack()

typedef struct {
  VENDOR_DEVICE_PATH       Custom;
  USB_DEVICE_PATH          Hub;
  USB_DEVICE_PATH          Dev;
  EFI_DEVICE_PATH_PROTOCOL EndDevicePath;
} PLATFORM_USB_DEV;

#define DW_USB_DXE_FILE_GUID                                                   \
  {                                                                            \
    0x4bf1704c, 0x03f4, 0x46d5,                                                \
    {                                                                          \
      0xbc, 0xa6, 0x82, 0xfa, 0x58, 0x0b, 0xad, 0xfd                           \
    }                                                                          \
  }

STATIC PLATFORM_USB_DEV mUsbHubPort = {
    //
    // VENDOR_DEVICE_PATH DwUsbHostDxe
    //
    {{HARDWARE_DEVICE_PATH, HW_VENDOR_DP, DP_NODE_LEN(VENDOR_DEVICE_PATH)},
     DW_USB_DXE_FILE_GUID},

    //
    // USB_DEVICE_PATH Hub
    //
    {{MESSAGING_DEVICE_PATH, MSG_USB_DP, DP_NODE_LEN(USB_DEVICE_PATH)}, 0, 0},

    //
    // USB_DEVICE_PATH Dev
    //
    {{MESSAGING_DEVICE_PATH, MSG_USB_DP, DP_NODE_LEN(USB_DEVICE_PATH)}, 1, 0},

    //
    // EFI_DEVICE_PATH_PROTOCOL End
    //
    {END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE,
     DP_NODE_LEN(EFI_DEVICE_PATH_PROTOCOL)}};

#define SERIAL_DXE_FILE_GUID                                                   \
  {                                                                            \
    0xD3987D4B, 0x971A, 0x435F,                                                \
    {                                                                          \
      0x8C, 0xAF, 0x49, 0x67, 0xEB, 0x62, 0x72, 0x41                           \
    }                                                                          \
  }

STATIC PLATFORM_SERIAL_CONSOLE mSerialConsole = {
    //
    // VENDOR_DEVICE_PATH SerialDxe
    //
    {{HARDWARE_DEVICE_PATH, HW_VENDOR_DP, DP_NODE_LEN(VENDOR_DEVICE_PATH)},
     SERIAL_DXE_FILE_GUID},

    //
    // UART_DEVICE_PATH Uart
    //
    {
        {MESSAGING_DEVICE_PATH, MSG_UART_DP, DP_NODE_LEN(UART_DEVICE_PATH)},
        0,                                     // Reserved
        FixedPcdGet64(PcdUartDefaultBaudRate), // BaudRate
        FixedPcdGet8(PcdUartDefaultDataBits),  // DataBits
        FixedPcdGet8(PcdUartDefaultParity),    // Parity
        FixedPcdGet8(PcdUartDefaultStopBits)   // StopBits
    },

    //
    // VENDOR_DEFINED_DEVICE_PATH TermType
    //
    {
        {MESSAGING_DEVICE_PATH, MSG_VENDOR_DP,
         DP_NODE_LEN(VENDOR_DEFINED_DEVICE_PATH)}
        //
        // Guid to be filled in dynamically
        //
    },

    //
    // EFI_DEVICE_PATH_PROTOCOL End
    //
    {END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE,
     DP_NODE_LEN(EFI_DEVICE_PATH_PROTOCOL)}};

#pragma pack(1)
typedef struct {
  USB_CLASS_DEVICE_PATH    Keyboard;
  EFI_DEVICE_PATH_PROTOCOL End;
} PLATFORM_USB_KEYBOARD;
#pragma pack()

STATIC PLATFORM_USB_KEYBOARD mUsbKeyboard = {
    //
    // USB_CLASS_DEVICE_PATH Keyboard
    //
    {
        {MESSAGING_DEVICE_PATH, MSG_USB_CLASS_DP,
         DP_NODE_LEN(USB_CLASS_DEVICE_PATH)},
        0xFFFF, // VendorId: any
        0xFFFF, // ProductId: any
        3,      // DeviceClass: HID
        1,      // DeviceSubClass: boot
        1       // DeviceProtocol: keyboard
    },

    //
    // EFI_DEVICE_PATH_PROTOCOL End
    //
    {END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE,
     DP_NODE_LEN(EFI_DEVICE_PATH_PROTOCOL)}};

/**
  Check if the handle satisfies a particular condition.

  @param[in] Handle      The handle to check.
  @param[in] ReportText  A caller-allocated string passed in for reporting
                         purposes. It must never be NULL.

  @retval TRUE   The condition is satisfied.
  @retval FALSE  Otherwise. This includes the case when the condition could not
                 be fully evaluated due to an error.
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
  INTN ShellOption;

  ShellOption = PlatformRegisterFvBootOption(
      &gUefiShellFileGuid, L"UEFI Shell", LOAD_OPTION_ACTIVE);
}

STATIC
VOID PlatformRegisterSetupKey(VOID)
{
  EFI_STATUS                   Status;
  EFI_INPUT_KEY                PowerBtn;
  EFI_BOOT_MANAGER_LOAD_OPTION BootOption;

  //
  // Map Power to Boot Manager Menu
  //
  PowerBtn.ScanCode    = SCAN_NULL;
  PowerBtn.UnicodeChar = CHAR_CARRIAGE_RETURN;
  Status               = EfiBootManagerGetBootManagerMenu(&BootOption);
  ASSERT_EFI_ERROR(Status);
  Status = EfiBootManagerAddKeyOptionVariable(
      NULL, (UINT16)BootOption.OptionNumber, 0, &PowerBtn, NULL);
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
  // Now add the device path of all handles with GOP on them to ConOut and
  // ErrOut.
  //
  FilterAndProcess(&gEfiGraphicsOutputProtocolGuid, NULL, AddOutput);

  //
  // Add the hardcoded short-form USB keyboard device path to ConIn.
  //
  EfiBootManagerUpdateConsoleVariable(
      ConIn, (EFI_DEVICE_PATH_PROTOCOL *)&mUsbKeyboard, NULL);

  //
  // Add touch screen to ConIn
  //
  EfiBootManagerUpdateConsoleVariable(
      ConIn, (EFI_DEVICE_PATH_PROTOCOL *)&TouchDxeDevicePath, NULL);

  //
  // Now add the device path of all handles with QcomKeypadDeviceProtocolGuid
  // on them to ConIn.
  //
  FilterAndProcess(&gEFIDroidKeypadDeviceProtocolGuid, NULL, AddInput);
  // Register setup key then
  PlatformRegisterSetupKey();

  //
  // Add the hardcoded serial console device path to ConIn, ConOut, ErrOut.
  //
  ASSERT(FixedPcdGet8(PcdDefaultTerminalType) == 4);
  CopyGuid(&mSerialConsole.TermType.Guid, &gEfiTtyTermGuid);

  EfiBootManagerUpdateConsoleVariable(
      ConIn, (EFI_DEVICE_PATH_PROTOCOL *)&mSerialConsole, NULL);
  EfiBootManagerUpdateConsoleVariable(
      ConOut, (EFI_DEVICE_PATH_PROTOCOL *)&mSerialConsole, NULL);
  EfiBootManagerUpdateConsoleVariable(
      ErrOut, (EFI_DEVICE_PATH_PROTOCOL *)&mSerialConsole, NULL);

  //
  // Signal EndOfDxe PI Event
  //
  EfiEventGroupSignal(&gEfiEndOfDxeEventGroupGuid);

  //
  // Dispatch deferred images after EndOfDxe event and ReadyToLock installation.
  //
  EfiBootManagerDispatchDeferredImages();
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
  ESRT_MANAGEMENT_PROTOCOL *EsrtManagement;
  EFI_STATUS                Status;

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

  //
  // Connect the rest of the devices.
  //
  EfiBootManagerConnectAll();

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

  PlatformRegisterOptionsAndKeys();
}

/**
  This function is called each second during the boot manager waits the
  timeout.

  @param TimeoutRemain  The remaining timeout.
**/
VOID EFIAPI PlatformBootManagerWaitCallback(UINT16 TimeoutRemain)
{
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION Black;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION White;
  UINT16                              Timeout;
  EFI_STATUS                          Status;

  Timeout = PcdGet16(PcdPlatformBootTimeOut);

  Black.Raw = 0x00000000;
  White.Raw = 0x00FFFFFF;

  Status = BootLogoUpdateProgress(
      White.Pixel, Black.Pixel, L"Hello from San Francisco\n", White.Pixel,
#if REQUIRE_PROGRESSBAR
      (Timeout - TimeoutRemain) * 100 / Timeout,
#else
      0,
#endif
      0);
  if (EFI_ERROR(Status)) {
    Print(L".");
  }
}
