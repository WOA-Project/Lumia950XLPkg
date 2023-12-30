#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomSmemPtableLib.h>
#include <Library/UefiBootServicesTableLib.h>

QCOM_SMEM_PTABLE_PROTOCOL *gSmemPtable = NULL;

RETURN_STATUS
EFIAPI
SmemLibConstructor(VOID)
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol(
      &gQcomSmemPtableProtocolGuid, NULL, (VOID **)&gSmemPtable);
  ASSERT_EFI_ERROR(Status);

  return Status;
}
