/* MpParkDxe: Microsoft ARM Multi-processor Parking Protocol */

#include <PiDxe.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/BaseLib.h>

#include <Library/ArmSmcLib.h>

EFI_STATUS
EFIAPI
MpParkDxeInitialize
(
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
{
    EFI Status = EFI_SUCCESS;

    // Run a SMC call to ensure Octacore enablement is supported
    

    return Status;
}