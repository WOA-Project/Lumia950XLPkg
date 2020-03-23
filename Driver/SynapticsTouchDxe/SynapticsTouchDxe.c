#include <Uefi.h>

#include <Library/LKEnvLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/AbsolutePointer.h>
#include <Protocol/QcomGpioTlmm.h>
#include <Protocol/QcomI2cQup.h>

#include "SynapticsRmi4.h"

// Instance Template
RMI4_INTERNAL_DATA mInstanceTemplate = {
    RMI4_TCH_INSTANCE_SIGNATURE,
    0,
    0,
    {
        AbsPReset,
        AbsPGetState,
        NULL,
        NULL,
    },
    {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    },
    NULL,
    FALSE,
    0,
    0,
    0,
    NULL,
    FALSE,
    NULL,
    NULL,
};

// Binding
EFI_DRIVER_BINDING_PROTOCOL gRmi4DriverBinding = {
    Rmi4AbsolutePointerDriverBindingSupported,
    Rmi4AbsolutePointerDriverBindingStart,
    Rmi4AbsolutePointerDriverBindingStop,
    0x1,
    NULL,
    NULL,
};

EFI_STATUS
EFIAPI
Rmi4AbsolutePointerDriverBindingSupported(
    IN EFI_DRIVER_BINDING_PROTOCOL *This, IN EFI_HANDLE Controller,
    IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath)
{
  EFI_STATUS            Status;
  SYNAPTICS_I2C_DEVICE *Rmi4I2cDeviceIo;

  Status = gBS->OpenProtocol(
      Controller, &gSynapticsTouchDeviceProtocolGuid, (VOID **)&Rmi4I2cDeviceIo,
      This->DriverBindingHandle, Controller, EFI_OPEN_PROTOCOL_BY_DRIVER);

  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = EFI_SUCCESS;
  gBS->CloseProtocol(
      Controller, &gSynapticsTouchDeviceProtocolGuid, This->DriverBindingHandle,
      Controller);

  return Status;
}

EFI_STATUS
EFIAPI
Rmi4AbsolutePointerDriverBindingStart(
    IN EFI_DRIVER_BINDING_PROTOCOL *This, IN EFI_HANDLE Controller,
    IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath)
{
  EFI_TPL               OldTpl;
  EFI_STATUS            Status;
  SYNAPTICS_I2C_DEVICE *Rmi4I2cDeviceIo;
  RMI4_INTERNAL_DATA *  Instance;

  UINT8 InfoData[TOUCH_RMI_PAGE_INFO_BYTES] = {0};
  UINT8 Address                             = TOUCH_RMI_PAGE_INFO_ADDRESS;

  UINT8 Page     = 0;
  UINT8 Function = 0;

  OldTpl = gBS->RaiseTPL(TPL_CALLBACK);
  Status = gBS->OpenProtocol(
      Controller, &gSynapticsTouchDeviceProtocolGuid, (VOID **)&Rmi4I2cDeviceIo,
      This->DriverBindingHandle, Controller, EFI_OPEN_PROTOCOL_BY_DRIVER);
  if (EFI_ERROR(Status)) {
    goto exit;
  }

  Instance = AllocateCopyPool(sizeof(RMI4_INTERNAL_DATA), &mInstanceTemplate);
  ASSERT(Instance != NULL);
  ASSERT(Rmi4I2cDeviceIo != NULL);

  Instance->Rmi4Device                  = Rmi4I2cDeviceIo;
  Instance->AbsPointerMode.AbsoluteMaxX = Rmi4I2cDeviceIo->XMax;
  Instance->AbsPointerMode.AbsoluteMaxY = Rmi4I2cDeviceIo->YMax;
  Instance->AbsPointerProtocol.Mode     = &Instance->AbsPointerMode;
  Instance->I2cController = Rmi4I2cDeviceIo->I2cQupProtocol->GetDevice(
      Rmi4I2cDeviceIo->ControllerI2cDevice);

  Status = SynaPowerUpController(Instance);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to power on controller \n"));
    goto exit;
  }

  DEBUG((EFI_D_INFO, "Probe Synaptics RMI4 F12 Function \n"));
  if (Instance->I2cController->suspended) {
    DEBUG((EFI_D_ERROR, "Device not found, or in invalid state \n"));
    Status = EFI_DEVICE_ERROR;
    goto exit;
  }

  // Probe device to locate F12 function
  do {
    Status =
        SynaI2cRead(Instance, Address, &InfoData[0], TOUCH_RMI_PAGE_INFO_BYTES);

    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "I2C Read failed \n"));
      goto exit;
    }

    if (InfoData[5] == TOUCH_RMI_F12_FUNCTION) {
      DEBUG((EFI_D_INFO, "RMI4 F12 Function Found. \n"));
      Instance->PageF12          = Page;
      Instance->TouchDataAddress = InfoData[3];
      break;
    }

    // Last page. Go out
    if (InfoData[5] == 0 && Address == TOUCH_RMI_PAGE_INFO_ADDRESS) {
      break;
    }
    // Switch page
    else if (InfoData[5] == 0 && Address != TOUCH_RMI_PAGE_INFO_ADDRESS) {
      DEBUG((EFI_D_INFO, "Switching to next Synaptics RMI4 Page........"));

      Page++;
      Address = TOUCH_RMI_PAGE_INFO_ADDRESS;

      Status = SynaI2cWrite(Instance, RMI_CHANGE_PAGE_ADDRESS, &Page, 1);
      if (EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "Unable to switch RMI4 page \n"));
        goto exit;
      }
    }
    else {
      Function++;
      Address = Address - TOUCH_RMI_PAGE_INFO_BYTES;
    }
  } while ((Address > 0) && (Function < TOUCH_RMI_MAX_FUNCTIONS));

  // Determine final result
  if (Function >= TOUCH_RMI_MAX_FUNCTIONS) {
    DEBUG((EFI_D_ERROR, "Invalid RMI4 function index \n"));
    Status = EFI_DEVICE_ERROR;
    goto exit;
  }

  if (Address <= 0) {
    DEBUG((EFI_D_ERROR, "Invalid RMI4 address \n"));
    Status = EFI_DEVICE_ERROR;
    goto exit;
  }

  // Flag device as initialized
  Instance->Initialized = TRUE;

  // Set touch event
  Status = gBS->CreateEvent(
      EVT_NOTIFY_WAIT, TPL_NOTIFY, AbsPWaitForInput, Instance,
      &Instance->AbsPointerProtocol.WaitForInput);
  ASSERT_EFI_ERROR(Status);

  // Start polling
  Status = AbsStartPolling(Instance);
  ASSERT_EFI_ERROR(Status);

  // Install protocols
  Status = gBS->InstallProtocolInterface(
      &Controller, &gEfiAbsolutePointerProtocolGuid, EFI_NATIVE_INTERFACE,
      &Instance->AbsPointerProtocol);

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to install protocol interface \n"));
  }

  // Name table
  Instance->ControllerNameTable = NULL;
  AddUnicodeString2(
      "eng", gRmi4DriverComponentName.SupportedLanguages,
      &Instance->ControllerNameTable, L"Synaptics RMI4 Absolute Pointer", TRUE);
  AddUnicodeString2(
      "en", gRmi4DriverComponentName.SupportedLanguages,
      &Instance->ControllerNameTable, L"Synaptics RMI4 Absolute Pointer",
      FALSE);

exit:
  gBS->RestoreTPL(OldTpl);
  return Status;
}

EFI_STATUS
EFIAPI
Rmi4AbsolutePointerDriverBindingStop(
    IN EFI_DRIVER_BINDING_PROTOCOL *This, IN EFI_HANDLE Controller,
    IN UINTN NumberOfChildren, IN EFI_HANDLE *ChildHandleBuffer)
{
  EFI_STATUS                     Status;
  EFI_ABSOLUTE_POINTER_PROTOCOL *AbsolutePointerProtocol;
  RMI4_INTERNAL_DATA *           Instance;

  Status = gBS->OpenProtocol(
      Controller, &gEfiAbsolutePointerProtocolGuid,
      (VOID **)&AbsolutePointerProtocol, This->DriverBindingHandle, Controller,
      EFI_OPEN_PROTOCOL_GET_PROTOCOL);

  if (EFI_ERROR(Status)) {
    return EFI_UNSUPPORTED;
  }

  Instance = RMI4_TCH_INSTANCE_FROM_ABSTCH_THIS(AbsolutePointerProtocol);

  Status = gBS->UninstallProtocolInterface(
      Controller, &gEfiAbsolutePointerProtocolGuid,
      &Instance->AbsPointerProtocol);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  gBS->CloseEvent(Instance->PollingTimerEvent);
  gBS->CloseEvent(Instance->AbsPointerProtocol.WaitForInput);

  return EFI_SUCCESS;
}

EFI_STATUS AbsPReset(
    IN EFI_ABSOLUTE_POINTER_PROTOCOL *This, IN BOOLEAN ExtendedVerification)
{
  RMI4_INTERNAL_DATA *Instance;

  Instance               = RMI4_TCH_INSTANCE_FROM_ABSTCH_THIS(This);
  Instance->LastX        = 0;
  Instance->LastY        = 0;
  Instance->StateChanged = FALSE;

  return EFI_SUCCESS;
}

EFI_STATUS AbsStartPolling(IN RMI4_INTERNAL_DATA *Instance)
{
  EFI_STATUS Status = EFI_SUCCESS;

  // Set event routines
  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL | EVT_TIMER, TPL_CALLBACK, SyncPollCallback, Instance,
      &Instance->PollingTimerEvent);
  ASSERT_EFI_ERROR(Status);

  Status = gBS->SetTimer(Instance->PollingTimerEvent, TimerPeriodic, 200000);
  ASSERT_EFI_ERROR(Status);

  return Status;
}

EFI_STATUS AbsPGetState(
    IN EFI_ABSOLUTE_POINTER_PROTOCOL *This,
    IN OUT EFI_ABSOLUTE_POINTER_STATE *State)
{
  EFI_STATUS          Status = EFI_SUCCESS;
  RMI4_INTERNAL_DATA *Instance;

  if (This == NULL || State == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  Instance = RMI4_TCH_INSTANCE_FROM_ABSTCH_THIS(This);
  if (!Instance->StateChanged) {
    Status = EFI_NOT_READY;
    goto exit;
  }

  State->CurrentX        = Instance->LastX;
  State->CurrentY        = Instance->LastY;
  State->CurrentZ        = 0;
  State->ActiveButtons   = 1;
  Instance->StateChanged = FALSE;

exit:
  return Status;
}

VOID EFIAPI AbsPWaitForInput(IN EFI_EVENT Event, IN VOID *Context)
{
  RMI4_INTERNAL_DATA *Instance = (RMI4_INTERNAL_DATA *)Context;
  EFI_TPL             OldTpl;

  //
  // Enter critical section
  //
  OldTpl = gBS->RaiseTPL(TPL_NOTIFY);

  SyncPollCallback(NULL, Instance);

  if (Instance->StateChanged) {
    gBS->SignalEvent(Event);
  }

  //
  // Leave critical section and return
  //
  gBS->RestoreTPL(OldTpl);
}

EFI_STATUS
EFIAPI
SynaPowerUpController(RMI4_INTERNAL_DATA *Instance)
{
  UINT32     ResetLine;
  EFI_STATUS Status;

  if (Instance == NULL || Instance->Rmi4Device == NULL ||
      Instance->Rmi4Device->GpioTlmmProtocol == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  // Pin Sanity check
  ResetLine = Instance->Rmi4Device->ControllerResetPin;
  if (ResetLine <= 0) {
    DEBUG((EFI_D_ERROR, "Invalid GPIO configuration \n"));
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  // Power Seq
  Instance->Rmi4Device->GpioTlmmProtocol->SetPull(ResetLine, GPIO_PULL_NONE);
  Instance->Rmi4Device->GpioTlmmProtocol->SetDriveStrength(ResetLine, 2);
  Instance->Rmi4Device->GpioTlmmProtocol->Set(ResetLine, GPIO_ENABLE);

  // Configure MSM GPIO RESET line to Low
  Instance->Rmi4Device->GpioTlmmProtocol->DirectionOutput(ResetLine, GPIO_LOW);
  gBS->Stall(TOUCH_POWER_RAIL_STABLE_TIME);

  // configure MSM GPIO RESET line to High
  Instance->Rmi4Device->GpioTlmmProtocol->DirectionOutput(ResetLine, GPIO_HIGH);
  gBS->Stall(TOUCH_DELAY_TO_COMMUNICATE);

  DEBUG((EFI_D_INFO, "Touch controller powered on \n"));
  Status = EFI_SUCCESS;

exit:
  return Status;
}

EFI_STATUS
EFIAPI
SyncGetTouchData(RMI4_INTERNAL_DATA *Instance, IN PTOUCH_DATA DataBuffer)
{
  EFI_STATUS Status                             = EFI_SUCCESS;
  UINT8      TouchCoordinates[TOUCH_DATA_BYTES] = {0};

  if (Instance == NULL || DataBuffer == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  if (!Instance->Initialized) {
    Status = EFI_NOT_READY;
    goto exit;
  }

  // Change RMI page to F12
  Status =
      SynaI2cWrite(Instance, RMI_CHANGE_PAGE_ADDRESS, &Instance->PageF12, 1);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to change RMI4 page address \n"));
    goto exit;
  }

  // Read a fingerprint
  Status = SynaI2cRead(
      Instance, Instance->TouchDataAddress, &TouchCoordinates[0],
      TOUCH_DATA_BYTES);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to read RMI4 F12 page data \n"));
    goto exit;
  }

  DataBuffer->TouchStatus = TouchCoordinates[0];
  DataBuffer->TouchX =
      ((TouchCoordinates[1] & 0xFF) | ((TouchCoordinates[2] & 0xFF) << 8));
  DataBuffer->TouchY =
      ((TouchCoordinates[3] & 0xFF) | ((TouchCoordinates[4] & 0xFF) << 8));

exit:
  return Status;
}

VOID EFIAPI SyncPollCallback(IN EFI_EVENT Event, IN VOID *Context)
{
  EFI_STATUS          Status;
  RMI4_INTERNAL_DATA *Instance = (RMI4_INTERNAL_DATA *)Context;
  TOUCH_DATA          TouchPointerData;
  EFI_TPL             OldTpl;

  //
  // Enter critical section
  //
  OldTpl = gBS->RaiseTPL(TPL_NOTIFY);

  Status = SyncGetTouchData(Instance, &TouchPointerData);

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to get Synaptics RMI4 F12 Data \n"));
  }
  else {
    if (TouchPointerData.TouchStatus > 0) {
      Instance->LastX        = TouchPointerData.TouchX;
      Instance->LastY        = TouchPointerData.TouchY;
      Instance->StateChanged = TRUE;

      DEBUG(
          (EFI_D_INFO, "Touch: X: %d, Y: %d \n", Instance->LastX,
           Instance->LastY));
    }
  }

  //
  // Leave critical section and return
  //
  gBS->RestoreTPL(OldTpl);
}

EFI_STATUS
EFIAPI
SynaInitialize(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  Status = EfiLibInstallDriverBindingComponentName2(
      ImageHandle, SystemTable, &gRmi4DriverBinding, ImageHandle,
      &gRmi4DriverComponentName, &gRmi4DriverComponentName2);
  ASSERT_EFI_ERROR(Status);

  return EFI_SUCCESS;
}
