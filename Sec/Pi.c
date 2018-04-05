// Pi.c: Entry point for SEC(Security).

#include <PiPei.h>
#include <Pi/PiBootMode.h>

#include <Pi/PiHob.h>
#include <Library/DebugLib.h>
#include <Library/PrePiLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Library/ArmLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/PrePiHobListPointerLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/DebugAgentLib.h>
#include <Ppi/GuidedSectionExtraction.h>
#include <Guid/LzmaDecompress.h>
#include <Library/SerialPortLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

#include <PiDxe.h>
#include "Pi.h"

VOID
EFIAPI
ProcessLibraryConstructorList
(
    VOID
);

STATIC VOID
UartInit
(
    VOID
)
{
    SerialPortInitialize();

    DEBUG((EFI_D_ERROR, "\nTianoCore on 950XL (AArch64)\n"));
    DEBUG((EFI_D_ERROR, "Firmware version %s built %a %a\n\n", 
        (CHAR16*) PcdGetPtr(PcdFirmwareVersionString), 
        __TIME__, 
        __DATE__
    ));
}

VOID
Main
(
    IN VOID  *StackBase,
    IN UINTN StackSize
)
{

    EFI_HOB_HANDOFF_INFO_TABLE*     HobList;
    EFI_STATUS                      Status;

    UINTN                           MemoryBase = 0;
    UINTN                           MemorySize = 0;
    UINTN                           StackBaseEx = 0;
    UINTN                           UefiMemoryBase = 0;
    UINTN                           UefiMemorySize = 0;

    // Architecture-specific initialization
    // Enable Floating Point
    ArmEnableVFP();

    /* Enable program flow prediction, if supported */
    ArmEnableBranchPrediction();

    // Initialize (fake) UART.
    UartInit();

    // Declare UEFI region
    MemoryBase      = FixedPcdGet32(PcdMemoryBase);
    MemorySize      = FixedPcdGet32(PcdMemorySize);
    UefiMemoryBase  = MemoryBase + FixedPcdGet32(PcdPreAllocatedMemorySize);
    UefiMemorySize  = FixedPcdGet32(PcdUefiMemPoolSize);
    StackBaseEx     = UefiMemoryBase + UefiMemorySize - 1;

    // Set up HOB
    HobList = HobConstructor(
        (VOID*) UefiMemoryBase,
        UefiMemorySize, 
        (VOID*) UefiMemoryBase, 
        (VOID*) StackBaseEx
    );

    PrePeiSetHobList(HobList);

    // Invalidate cache
    InvalidateDataCacheRange(
        (VOID *) (UINTN) PcdGet64 (PcdFdBaseAddress), 
        PcdGet32 (PcdFdSize)
    );

    // Initialize MMU
    Status = MemoryPeim(
        UefiMemoryBase, 
        UefiMemorySize
    );
    ASSERT_EFI_ERROR(Status);

    // Add HOBs
    BuildStackHob((UINTN) StackBase, StackSize);

    // Set the Boot Mode
    SetBootMode(BOOT_WITH_FULL_CONFIGURATION);

    // Initialize Platform HOBs (CpuHob and FvHob)
    Status = PlatformPeim();
    ASSERT_EFI_ERROR(Status);

    // SEC phase needs to run library constructors by hand.
    ProcessLibraryConstructorList();

    // Assume the FV that contains the PI (our code) also contains a compressed FV.
    Status = DecompressFirstFv();
    ASSERT_EFI_ERROR(Status);

    // Load the DXE Core and transfer control to it
    Status = LoadDxeCoreFromFv(NULL, 0);
    ASSERT_EFI_ERROR(Status);
    
    // We should never reach here
    CpuDeadLoop();
}

VOID
CEntryPoint
(
    IN  VOID  *StackBase,
    IN  UINTN StackSize
)
{
    Main(StackBase, StackSize);
}