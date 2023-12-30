#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomBoardLib.h>
#include <Library/QcomSmemLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS
EFIAPI
BoardDxeInitialize(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_HANDLE Handle = NULL;
  EFI_STATUS Status;

  BoardImplLibInitialize();

  Status = gBS->InstallMultipleProtocolInterfaces(
      &Handle, &gQcomBoardProtocolGuid, gBoard, NULL);
  ASSERT_EFI_ERROR(Status);

  return Status;
}
