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

EFI_STATUS
EFIAPI
SynaI2cRead(
    RMI4_INTERNAL_DATA *Instance, IN UINT8 Address, IN UINT8 *Data,
    IN UINT16 ReadBytes)
{
  EFI_STATUS Status      = EFI_SUCCESS;
  int        Transferred = 0;

  if (Instance == NULL || Instance->Rmi4Device == NULL ||
      Instance->I2cController == NULL ||
      Instance->Rmi4Device->I2cQupProtocol == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  if (Data == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  struct i2c_msg ControllerProbeMsg[] = {
      {
          Instance->Rmi4Device->ControllerAddress,
          I2C_M_WR,
          sizeof(UINT8),
          (UINT8 *)&Address,
      },
      {
          Instance->Rmi4Device->ControllerAddress,
          I2C_M_RD,
          ReadBytes,
          Data,
      },
  };

  Transferred = Instance->Rmi4Device->I2cQupProtocol->Transfer(
      Instance->I2cController, ControllerProbeMsg, 2);
  if (Transferred != 2)
    Status = EFI_DEVICE_ERROR;

exit:
  return Status;
}

EFI_STATUS
EFIAPI
SynaI2cWrite(
    RMI4_INTERNAL_DATA *Instance, IN UINT8 Address, IN UINT8 *Data,
    IN UINT16 WriteBytes)
{
  EFI_STATUS Status      = EFI_SUCCESS;
  int        Transferred = 0;

  if (Instance == NULL || Instance->Rmi4Device == NULL ||
      Instance->I2cController == NULL ||
      Instance->Rmi4Device->I2cQupProtocol == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  if (Data == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  struct i2c_msg ControllerProbeMsg[] = {
      {
          Instance->Rmi4Device->ControllerAddress,
          I2C_M_WR,
          sizeof(UINT8),
          (UINT8 *)&Address,
      },
      {
          Instance->Rmi4Device->ControllerAddress,
          I2C_M_WR,
          WriteBytes,
          Data,
      },
  };

  Transferred = Instance->Rmi4Device->I2cQupProtocol->Transfer(
      Instance->I2cController, ControllerProbeMsg, 2);
  if (Transferred != 2)
    Status = EFI_DEVICE_ERROR;

exit:
  return Status;
}
