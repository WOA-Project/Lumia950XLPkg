// Pi.c: Entry point for SEC(Security).

#include <PiPei.h>

#include <Pi/PiHob.h>
#include <Library/DebugLib.h>
#include <Library/PrePiLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Library/ArmLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/DebugAgentLib.h>
#include <Ppi/GuidedSectionExtraction.h>
#include <Guid/LzmaDecompress.h>
#include <Library/SerialPortLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

#include <Library/ProcAsmLib.h>
#include <Library/QcomBaseLib.h>
#include <Library/FBPTLib.h>

#include <PiDxe.h>
// #include <PreInitializeVariableInfo.h>

#ifndef _BGRA8888_COLORS_
#define _BGRA8888_COLORS_
#define BGRA8888_BLACK          0xff000000
#define BGRA8888_WHITE          0xffffffff
#define BGRA8888_CYAN           0xff00ffff
#define BGRA8888_BLUE           0xff0000ff
#define BGRA8888_SILVER         0xffc0c0c0
#define BGRA8888_YELLOW         0xffffff00
#define BGRA8888_ORANGE         0xffffa500
#define BGRA8888_RED            0xffff0000
#define BGRA8888_GREEN          0xff00ff00
#endif

#ifndef _FB_ADDRESS_
#define _FB_ADDRESS_
#define FB_ADDR                 0x400000
#endif

STATIC VOID
UartInit
(
    VOID
)
{
    UINT32 AbsTimems;
    AbsTimems = GetTimerCountms();
    SerialPortInitialize();
    DEBUG ((EFI_D_ERROR, "\nTianoCore on 950XL (AArch64)\n", AbsTimems));
    DEBUG ((EFI_D_ERROR, "UEFI Start : %d ms\n\n", AbsTimems));
}

/* Initialize the cycle counter to track performance */
STATIC VOID
StartCyclCounter
(
    VOID
)
{
    UINTN RegVal;
    UINT64 Val;
    UINT64 Scale;
    UINT32 AppsProcClkMhz;

    /* User mode enable to read in non secure mode */
    WriteUserEnReg (1);

    /* Reset counters */
    RegVal = (0x41 << 24) |  /* IMP */
            (4 << 11)    |  /* N */
            (1 << 3)     |  /* 1/64 */
            (1 << 2);       /* Reset CCNT */
    WritePMonCtlReg (RegVal);

    ReadCycleCntReg();

    /* Scale bootcounter running at 32KHz to CPU frequency in MHz, counting every 64 cycles.
        Get the Scale to be accurate to 3 decimal places by multiplying it with 2^10.  
        Then divide by 2^10 (right shift by 10) to nullify the multiplication done before and 
        divide by 64 (Right shift 6) => Right shift by 16 to set Cycle Counter Start Value */
    AppsProcClkMhz = PcdGet32(PcdAppsProcFrequencyMhz);
    Scale = (AppsProcClkMhz << 10) / 1000;
    Val =  Scale * BootGetTimeNanoSec();
    Val = (Val >> 16);
    WriteCycleCntReg((UINT32)Val);

    /* Check if write went through */
    ReadCycleCntReg();

    /* Enable Cycle counter */
    WriteCntEnSetReg (((UINT32)1 << 31));

    /* Check if we start counting */
    ReadCycleCntReg();

    /* Enable CCNT */
    RegVal = (0x41 << 24) |     /* IMP */
            (4 << 11)    |      /* N */
            (1 << 3)     |      /* 1/64 */
            (1);                /* Enable all counters */
    WritePMonCtlReg(RegVal);

    /* Disable User mode access */
    WriteUserEnReg(0);

    /* Write to TPIDRURW */
    WriteTPIDRURWReg(0x56430000);

    /* Write to TPIDRURO */
    WriteTPIDRUROReg(0);

    /* Example to Read the counter value, Should read small */
    ReadCycleCntReg();
}

VOID
Main
(
    IN VOID  *StackBase,
    IN UINTN StackSize
)
{

    // Initialize UART.
    UartInit();

    // We are done
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