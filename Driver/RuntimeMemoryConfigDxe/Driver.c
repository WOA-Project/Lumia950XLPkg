#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/RuntimeSpmiMemoryConfig.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS
EFIAPI
RtMemoryConfigEntry(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  // Config runtime SPMI memory regions
  Status = RtSpmiConfigMemory();

  // This must success
  if (EFI_ERROR(Status)) {
    DEBUG(
        (EFI_D_ERROR | EFI_D_INFO,
         "Unable to configure runtime memory. Check PEI memory config. \n"));
    CpuDeadLoop();
  }

  return Status;
}