#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/QcomRpmLib.h>
#include <Library/UefiBootServicesTableLib.h>

QCOM_RPM_PROTOCOL *gRpm = NULL;

RETURN_STATUS
EFIAPI
RpmLibConstructor (
  VOID
  )
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol (&gQcomRpmProtocolGuid, NULL, (VOID **)&gRpm);
  ASSERT_EFI_ERROR (Status);

  return Status;
}
