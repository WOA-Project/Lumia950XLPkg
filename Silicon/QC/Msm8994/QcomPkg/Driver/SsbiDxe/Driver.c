#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomSsbiLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS
EFIAPI
SsbiDxeInitialize(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_HANDLE Handle = NULL;
  EFI_STATUS Status;

  SsbiImplLibInitialize();

  Status = gBS->InstallMultipleProtocolInterfaces(
      &Handle, &gQcomSsbiProtocolGuid, gSSBI, NULL);
  ASSERT_EFI_ERROR(Status);

  return Status;
}
