#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomSsbiLib.h>
#include <Library/UefiBootServicesTableLib.h>

QCOM_SSBI_PROTOCOL *gSSBI = NULL;

RETURN_STATUS
EFIAPI
SsbiLibConstructor(VOID)
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol(&gQcomSsbiProtocolGuid, NULL, (VOID **)&gSSBI);
  ASSERT_EFI_ERROR(Status);

  return Status;
}
