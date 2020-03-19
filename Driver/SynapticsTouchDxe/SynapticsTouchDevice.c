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

SYNAPTICS_I2C_DEVICE mTemplate = {
    RMI4_DEV_INSTANCE_SIGNATURE, 0, 0, 0, 0, 0, 0, NULL, NULL,
};

EFI_STATUS EFIAPI SynaDeviceInitialize(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  SYNAPTICS_I2C_DEVICE *Instance;
  EFI_STATUS            Status;

  // Device instance
  Instance = AllocateCopyPool(sizeof(SYNAPTICS_I2C_DEVICE), &mTemplate);
  if (Instance == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto exit;
  }

  // Config
  Instance->ControllerI2cDevice    = FixedPcdGet32(SynapticsCtlrI2cDevice);
  Instance->ControllerAddress      = FixedPcdGet16(SynapticsCtlrAddress);
  Instance->ControllerResetPin     = FixedPcdGet32(SynapticsCtlrResetPin);
  Instance->ControllerInterruptPin = FixedPcdGet32(SynapticsCtlrIntPin);
  Instance->XMax                   = FixedPcdGet32(SynapticsXMax);
  Instance->YMax                   = FixedPcdGet32(SynapticsYMax);

  // Protocol
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

  // Looks good and publish the protocol
  Status = gBS->InstallMultipleProtocolInterfaces(
      &ImageHandle, &gSynapticsTouchDeviceProtocolGuid, Instance,
      &gEfiDevicePathProtocolGuid, &TouchDxeDevicePath, NULL);

exit:
  return Status;
}
