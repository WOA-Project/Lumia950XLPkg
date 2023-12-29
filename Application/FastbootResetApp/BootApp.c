#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

EFI_STATUS EFIAPI FbResetEntry(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    gRT->ResetSystem(EfiResetWarm, EFI_MEDIA_CHANGED, 0, 0);
    return EFI_ABORTED;
}
