#include <Guid/EventGroup.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h> // gRT

#include <Protocol/Cpu.h>
#include <Protocol/DevicePathFromText.h>
#include <Protocol/EmbeddedGpio.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/PlatformBootManager.h>

#include <Library/DeviceStateLib.h>

#include <Guid/GlobalVariable.h>

/**
  Helper function to query whether the secure boot variable is in place.
  For Project Mu Code if the PK is set then Secure Boot is enforced (there is no
  SetupMode)

  @retval     TRUE if secure boot is enabled, FALSE otherwise.
**/
BOOLEAN
IsSecureBootOn()
{
  EFI_STATUS Status;
  UINTN      PkSize = 0;

  Status = gRT->GetVariable(
      EFI_PLATFORM_KEY_NAME, &gEfiGlobalVariableGuid, NULL, &PkSize, NULL);
  if ((Status == EFI_BUFFER_TOO_SMALL) && (PkSize > 0)) {
    DEBUG(
        (DEBUG_INFO, "%a - PK exists.  Secure boot on.  Pk Size is 0x%X\n",
         __FUNCTION__, PkSize));
    return TRUE;
  }

  DEBUG(
      (DEBUG_INFO, "%a - PK doesn't exist.  Secure boot off\n", __FUNCTION__));
  return FALSE;
}

EFI_STATUS
EFIAPI
InitializeColorbars(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  DEVICE_STATE State;

  State = 0;

  if (!IsSecureBootOn()) {
    State |= DEVICE_STATE_SECUREBOOT_OFF;
  }

  State |= DEVICE_STATE_PLATFORM_MODE_0;
  State |= DEVICE_STATE_DEVELOPMENT_BUILD_ENABLED;
  State |= DEVICE_STATE_MANUFACTURING_MODE;

#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1 || USE_SCREEN_FOR_SERIAL_OUTPUT == 1
  State |= DEVICE_STATE_UNIT_TEST_MODE;
#endif

  AddDeviceState(State);

  return EFI_SUCCESS;
}