#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/QcomSpmiLib.h>
#include <Library/UefiBootServicesTableLib.h>

QCOM_SPMI_PROTOCOL *gSpmi = NULL;

RETURN_STATUS
EFIAPI
SpmiLibConstructor (
  VOID
  )
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol (&gQcomSpmiProtocolGuid, NULL, (VOID **)&gSpmi);
  ASSERT_EFI_ERROR (Status);

  return Status;
}
