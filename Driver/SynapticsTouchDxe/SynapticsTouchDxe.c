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
#include <Device/TouchDevicePath.h>

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
        FixedPcdGet64(SynapticsXMax),
        FixedPcdGet64(SynapticsYMax),
        0,
        0,
    },
    NULL,
    FALSE,
    0,
    0,
    0,
    NULL,
    NULL,
    NULL,
};

EFI_STATUS AbsPReset(
    IN EFI_ABSOLUTE_POINTER_PROTOCOL *This, IN BOOLEAN ExtendedVerification)
{
  RMI4_INTERNAL_DATA *Instance;

  Instance        = RMI4_TCH_INSTANCE_FROM_BLOCKIO_THIS(This);
  Instance->LastX = 0;
  Instance->LastY = 0;

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

  Status = gBS->SetTimer(
      Instance->PollingTimerEvent, TimerPeriodic,
      EFI_TIMER_PERIOD_MILLISECONDS(25));
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

  Instance = RMI4_TCH_INSTANCE_FROM_BLOCKIO_THIS(This);

  State->CurrentX      = Instance->LastX;
  State->CurrentY      = Instance->LastY;
  State->CurrentZ      = 0;
  State->ActiveButtons = 1;

exit:
  return Status;
}

VOID EFIAPI AbsPWaitForInput(IN EFI_EVENT Event, IN VOID *Context)
{
  // Nothing to do
}

EFI_STATUS
EFIAPI
SynaPowerUpController(RMI4_INTERNAL_DATA *Instance)
{
  UINT32     ResetLine;
  EFI_STATUS Status;

  if (Instance == NULL || Instance->GpioTlmmProtocol == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  // Pin Sanity check
  ResetLine = FixedPcdGet32(SynapticsCtlrResetPin);
  if (ResetLine <= 0) {
    DEBUG((EFI_D_ERROR, "Invalid GPIO configuration \n"));
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  // Power Seq
  Instance->GpioTlmmProtocol->SetPull(ResetLine, GPIO_PULL_NONE);
  Instance->GpioTlmmProtocol->SetDriveStrength(ResetLine, 2);
  Instance->GpioTlmmProtocol->Set(ResetLine, GPIO_ENABLE);

  // Configure MSM GPIO RESET line to Low
  Instance->GpioTlmmProtocol->DirectionOutput(ResetLine, GPIO_LOW);
  gBS->Stall(TOUCH_POWER_RAIL_STABLE_TIME);

  // configure MSM GPIO RESET line to High
  Instance->GpioTlmmProtocol->DirectionOutput(ResetLine, GPIO_HIGH);
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

  DEBUG((EFI_D_ERROR, "-> SyncGetTouchData\n"));
  Status = SyncGetTouchData(Instance, &TouchPointerData);
  DEBUG((EFI_D_ERROR, "<- SyncGetTouchData\n"));

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Faild to get Synaptics RMI4 F12 Data \n"));
  }
  else {
    if (TouchPointerData.TouchStatus > 0) {
      Instance->LastX = TouchPointerData.TouchX;
      Instance->LastY = TouchPointerData.TouchY;
      gBS->SignalEvent(Instance->AbsPointerProtocol.WaitForInput);

      DEBUG(
          (EFI_D_INFO | EFI_D_ERROR, "Touch: X: %d, Y: %d \n", Instance->LastX,
           Instance->LastY));
    }
  }
}

EFI_STATUS
EFIAPI
SynaInitialize(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS          Status = EFI_SUCCESS;
  RMI4_INTERNAL_DATA *Instance;

  UINT8  InfoData[TOUCH_RMI_PAGE_INFO_BYTES] = {0};
  UINT8  Address                             = TOUCH_RMI_PAGE_INFO_ADDRESS;
  UINTN  DeviceIndex                         = 0;
  UINT16 ControllerSlaveAddr                 = 0;

  UINT8 Page     = 0;
  UINT8 Function = 0;

  // Device instance
  Instance = AllocateCopyPool(sizeof(RMI4_INTERNAL_DATA), &mInstanceTemplate);
  if (Instance == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto exit;
  }

  Instance->AbsPointerProtocol.Mode = &Instance->AbsPointerMode;

  // Device ID
  DeviceIndex = FixedPcdGet32(SynapticsCtlrI2cDevice);
  if (DeviceIndex <= 0) {
    DEBUG((EFI_D_ERROR, "Invalid I2C Device \n"));
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  // Controller Slave Address
  ControllerSlaveAddr = FixedPcdGet16(SynapticsCtlrAddress);
  if (ControllerSlaveAddr <= 0) {
    DEBUG((EFI_D_ERROR, "Invalid I2C Address \n"));
    Status = EFI_INVALID_PARAMETER;
  }

  // Locate protocol
  Status = gBS->LocateProtocol(
      &gQcomI2cQupProtocolGuid, NULL, (VOID *)&Instance->I2cQupProtocol);

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Unable to locate I2C protocol \n"));
    goto exit;
  }

  // GPIO Processing
  Status = gBS->LocateProtocol(
      &gQcomGpioTlmmProtocolGuid, NULL, (VOID *)&Instance->GpioTlmmProtocol);

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Unable to locate GPIO protocol \n"));
    goto exit;
  }

  // Power Seq
  Status = SynaPowerUpController(Instance);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to power on controller \n"));
    goto exit;
  }

  DEBUG((EFI_D_INFO, "Probe Synaptics RMI4 F12 Function \n"));

  // Locate device
  Instance->I2cController = Instance->I2cQupProtocol->GetDevice(DeviceIndex);

  if (Instance->I2cController != NULL && !Instance->I2cController->suspended) {
    // Probe device to locate F12 function
    do {
      Status = SynaI2cRead(
          Instance, Address, &InfoData[0], TOUCH_RMI_PAGE_INFO_BYTES);

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
        EVT_NOTIFY_WAIT, TPL_NOTIFY, AbsPWaitForInput, NULL,
        &Instance->AbsPointerProtocol.WaitForInput);
    ASSERT_EFI_ERROR(Status);

    // Start polling
    Status = AbsStartPolling(Instance);
    ASSERT_EFI_ERROR(Status);

    // Install protocols
    Status = gBS->InstallMultipleProtocolInterfaces(
        &ImageHandle, &gEfiAbsolutePointerProtocolGuid,
        &Instance->AbsPointerProtocol, &gEfiDevicePathProtocolGuid,
        &TouchDxeDevicePath, NULL);

    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Failed to install protocol interface \n"));
    }
  }
  else {
    DEBUG((EFI_D_ERROR, "Device not found, or in invalid state \n"));
    Status = EFI_DEVICE_ERROR;
  }

exit:
  ASSERT_EFI_ERROR(Status);
  return Status;
}
