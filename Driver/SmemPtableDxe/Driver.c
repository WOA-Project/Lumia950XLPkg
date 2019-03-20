#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomSmemPtableLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS
EFIAPI
SmemPtableDxeInitialize(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_HANDLE Handle = NULL;
  EFI_STATUS Status;

  SmemPtableImplLibInitialize();

  Status = gBS->InstallMultipleProtocolInterfaces(
      &Handle, &gQcomSmemPtableProtocolGuid, gSmemPtable, NULL);
  ASSERT_EFI_ERROR(Status);

  return Status;
}
