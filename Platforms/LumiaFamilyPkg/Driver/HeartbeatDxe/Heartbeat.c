/* Heartbeat: periodically flash the LED to indicate firmware status */

#include <PiDxe.h>
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

static EFI_EVENT m_CallbackTimer = NULL;

VOID EFIAPI LedHeartbeatCallback(IN EFI_EVENT Event, IN VOID *Context)
{
  DEBUG((EFI_D_ERROR, "Heartbeat \n"));
}

EFI_STATUS
EFIAPI
HeartbeatInitialize(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL | EVT_TIMER, TPL_CALLBACK, LedHeartbeatCallback, NULL,
      &m_CallbackTimer);

  ASSERT_EFI_ERROR(Status);

  Status = gBS->SetTimer(
      m_CallbackTimer, TimerPeriodic, EFI_TIMER_PERIOD_MILLISECONDS(500));

  ASSERT_EFI_ERROR(Status);
  return Status;
}
