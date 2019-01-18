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
#include <Library/PerformanceLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/DebugAgentLib.h>
#include <Ppi/GuidedSectionExtraction.h>
#include <Guid/LzmaDecompress.h>
#include <Library/SerialPortLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/FrameBufferSerialPortLib.h>

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
    IN VOID     *StackBase,
    IN UINTN    StackSize,
    IN UINT64   StartTimeStamp
)
{

    EFI_HOB_HANDOFF_INFO_TABLE*     HobList;
    EFI_STATUS                      Status;

    UINTN                           MemoryBase = 0;
    UINTN                           MemorySize = 0;
    UINTN                           UefiMemoryBase = 0;
    UINTN                           UefiMemorySize = 0;

    // Architecture-specific initialization
    // Enable Floating Point
    ArmEnableVFP();

    /* Enable program flow prediction, if supported */
    ArmEnableBranchPrediction();

    // Clear FB
    ResetFb();

    // Initialize (fake) UART.
    UartInit();

    // Declare UEFI region
    MemoryBase      = FixedPcdGet32(PcdSystemMemoryBase);
    MemorySize      = FixedPcdGet32(PcdSystemMemorySize);
    UefiMemoryBase  = MemoryBase + FixedPcdGet32(PcdPreAllocatedMemorySize);
    UefiMemorySize  = FixedPcdGet32(PcdUefiMemPoolSize);
    StackBase       = (VOID*) (UefiMemoryBase + UefiMemorySize - StackSize);

    DEBUG((
        EFI_D_INFO | EFI_D_LOAD, 
        "UEFI Memory Base = 0x%llx, Size = 0x%llx, Stack Base = 0x%llx, Stack Size = 0x%llx\n",
        UefiMemoryBase,
        UefiMemorySize,
        StackBase,
        StackSize
    ));

    // Set up HOB
    HobList = HobConstructor(
        (VOID*) UefiMemoryBase,
        UefiMemorySize, 
        (VOID*) UefiMemoryBase, 
        StackBase
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

    if (EFI_ERROR(Status))
    {
        DEBUG((EFI_D_ERROR, "Failed to configure MMU\n"));
        CpuDeadLoop();
    }

    DEBUG((EFI_D_LOAD | EFI_D_INFO, "MMU configured from device config\n"));

	// Initialize GIC
	Status = QGicPeim();

	if (EFI_ERROR(Status))
	{
		DEBUG((EFI_D_ERROR, "Failed to configure GIC\n"));
		CpuDeadLoop();
	}

    // Add HOBs
    BuildStackHob((UINTN) StackBase, StackSize);

    //TODO: Call CpuPei as a library
    BuildCpuHob(PcdGet8(PcdPrePiCpuMemorySize), PcdGet8(PcdPrePiCpuIoSize));

    // Set the Boot Mode
    SetBootMode(BOOT_WITH_FULL_CONFIGURATION);

    // Initialize Platform HOBs (CpuHob and FvHob)
    Status = PlatformPeim();
    ASSERT_EFI_ERROR(Status);

    // Now, the HOB List has been initialized, we can register performance information
    // PERF_START (NULL, "PEI", NULL, StartTimeStamp);

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
    Main(StackBase, StackSize, 0);
}