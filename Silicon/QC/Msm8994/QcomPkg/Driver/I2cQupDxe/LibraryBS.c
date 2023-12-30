#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomI2cQupLib.h>
#include <Library/UefiBootServicesTableLib.h>

QCOM_I2C_QUP_PROTOCOL *gI2cQup = NULL;

RETURN_STATUS
EFIAPI
I2cQupLibConstructor(VOID)
{
  EFI_STATUS Status;

  Status =
      gBS->LocateProtocol(&gQcomI2cQupProtocolGuid, NULL, (VOID **)&gI2cQup);
  ASSERT_EFI_ERROR(Status);

  return Status;
}
