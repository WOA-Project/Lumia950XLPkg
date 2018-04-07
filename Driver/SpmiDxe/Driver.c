#include <PiDxe.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/QcomSpmiLib.h>
#include <Library/LKEnvLib.h>

EFI_STATUS
EFIAPI
SpmiDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_HANDLE Handle = NULL;
  EFI_STATUS Status;

  SpmiImplLibInitialize();

  Status = gBS->InstallMultipleProtocolInterfaces(
                  &Handle,
                  &gQcomSpmiProtocolGuid,      gSpmi,
                  NULL
                  );
  ASSERT_EFI_ERROR(Status);

  return Status;
}
