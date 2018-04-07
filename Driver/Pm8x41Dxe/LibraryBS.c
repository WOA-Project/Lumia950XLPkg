#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/QcomPm8x41Lib.h>
#include <Library/UefiBootServicesTableLib.h>

QCOM_PM8X41_PROTOCOL *gPm8x41 = NULL;

RETURN_STATUS
EFIAPI
Pm8x41LibConstructor (
  VOID
  )
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol (&gQcomPm8x41ProtocolGuid, NULL, (VOID **)&gPm8x41);
  ASSERT_EFI_ERROR (Status);

  return Status;
}
