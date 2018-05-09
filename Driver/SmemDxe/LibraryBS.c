#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/QcomSmemLib.h>
#include <Library/UefiBootServicesTableLib.h>

QCOM_SMEM_PROTOCOL *gSMEM = NULL;

RETURN_STATUS
EFIAPI
SmemLibConstructor (
  VOID
  )
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol (&gQcomSmemProtocolGuid, NULL, (VOID **)&gSMEM);
  ASSERT_EFI_ERROR (Status);

  return Status;
}
