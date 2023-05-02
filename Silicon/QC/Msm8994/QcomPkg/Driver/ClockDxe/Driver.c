#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomClockLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS
EFIAPI
ClockDxeInitialize(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_HANDLE Handle = NULL;
  EFI_STATUS Status;

  ClockImplLibInitialize();

  Status = gBS->InstallMultipleProtocolInterfaces(
      &Handle, &gQcomClockProtocolGuid, gClock, NULL);
  ASSERT_EFI_ERROR(Status);

  return Status;
}
