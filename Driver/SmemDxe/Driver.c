#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/QcomSmemLib.h>

EFI_STATUS
EFIAPI
SmemDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_HANDLE Handle = NULL;
  EFI_STATUS Status;

  SmemImplLibInitialize();

  Status = gBS->InstallMultipleProtocolInterfaces(
                  &Handle,
                  &gQcomSmemProtocolGuid,      gSMEM,
                  NULL
                  );
  ASSERT_EFI_ERROR(Status);

  return Status;
}
