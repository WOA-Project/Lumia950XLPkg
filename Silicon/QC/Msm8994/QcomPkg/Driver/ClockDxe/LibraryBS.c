#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomClockLib.h>
#include <Library/UefiBootServicesTableLib.h>

QCOM_CLOCK_PROTOCOL *gClock = NULL;

RETURN_STATUS
EFIAPI
ClockLibConstructor(VOID)
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol(&gQcomClockProtocolGuid, NULL, (VOID **)&gClock);
  ASSERT_EFI_ERROR(Status);

  return Status;
}
