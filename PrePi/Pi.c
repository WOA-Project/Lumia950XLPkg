// Pi.c: Entry point for SEC(Security).

#include "Pi.h"

#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <PiDxe.h>
#include <PiPei.h>

#include <Configuration/Hob.h>
#include <Guid/LzmaDecompress.h>
#include <Ppi/GuidedSectionExtraction.h>

#include <Library/ArmGicLib.h>
#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/DebugLib.h>
#include <Library/FrameBufferSerialPortLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PrePiHobListPointerLib.h>
#include <Library/PrePiLib.h>
#include <Library/SerialPortLib.h>

#include <Library/ArmHvcLib.h>
#include <Library/ArmSmcLib.h>

#include <IndustryStandard/ArmStdSmc.h>

VOID EFIAPI ProcessLibraryConstructorList(VOID);
extern void SecondaryCpuEntry();

static UINT32 ProcessorIdMapping[8] = {
    0x00000000, 0x00000001, 0x00000002, 0x00000003,
    0x00000100, 0x00000101, 0x00000102, 0x00000103,
};

STATIC VOID UartInit(VOID)
{
  SerialPortInitialize();

  DEBUG((EFI_D_INFO, "\nTianoCore on 950XL (AArch64)\n"));
  DEBUG(
      (EFI_D_INFO, "Firmware version %s built %a %a\n\n",
       (CHAR16 *)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__));
}

VOID Main(IN VOID *StackBase, IN UINTN StackSize, IN UINT64 StartTimeStamp)
{

  EFI_HOB_HANDOFF_INFO_TABLE *HobList;
  PRELOADER_ENVIRONMENT *     PreEnv = (VOID *)PRELOADER_ENV_ADDR;
  UINT32                      Crc32  = 0;
  EFI_STATUS                  Status;

  UINTN MemoryBase     = 0;
  UINTN MemorySize     = 0;
  UINTN UefiMemoryBase = 0;
  UINTN UefiMemorySize = 0;

  // Architecture-specific initialization
  // Enable Floating Point
  ArmEnableVFP();

  /* Enable program flow prediction, if supported */
  ArmEnableBranchPrediction();

  // Initialize (fake) UART.
  UartInit();

  // Declare UEFI region
  MemoryBase     = FixedPcdGet32(PcdSystemMemoryBase);
  MemorySize     = FixedPcdGet32(PcdSystemMemorySize);
  UefiMemoryBase = MemoryBase + FixedPcdGet32(PcdPreAllocatedMemorySize);
  UefiMemorySize = FixedPcdGet32(PcdUefiMemPoolSize);
  StackBase      = (VOID *)(UefiMemoryBase + UefiMemorySize - StackSize);

  DEBUG(
      (EFI_D_INFO | EFI_D_LOAD,
       "UEFI Memory Base = 0x%llx, Size = 0x%llx, Stack Base = 0x%llx, Stack "
       "Size = 0x%llx\n",
       UefiMemoryBase, UefiMemorySize, StackBase, StackSize));

  // Set up HOB
  HobList = HobConstructor(
      (VOID *)UefiMemoryBase, UefiMemorySize, (VOID *)UefiMemoryBase,
      StackBase);

  PrePeiSetHobList(HobList);

  // Invalidate cache
  InvalidateDataCacheRange(
      (VOID *)(UINTN)PcdGet64(PcdFdBaseAddress), PcdGet32(PcdFdSize));

  // Initialize MMU
  Status = MemoryPeim(UefiMemoryBase, UefiMemorySize);

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to configure MMU\n"));
    CpuDeadLoop();
  }

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "MMU configured from device config\n"));

  // Initialize GIC
  if (!FixedPcdGetBool(PcdIsLkBuild)) {
    Status = QGicPeim();

    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Failed to configure GIC\n"));
      CpuDeadLoop();
    }
  }

  // Add HOBs
  BuildStackHob((UINTN)StackBase, StackSize);

  // TODO: Call CpuPei as a library
  BuildCpuHob(40, PcdGet8(PcdPrePiCpuIoSize));

  // Set the Boot Mode
  SetBootMode(BOOT_WITH_FULL_CONFIGURATION);

  // Initialize Platform HOBs (CpuHob and FvHob)
  Status = PlatformPeim();
  ASSERT_EFI_ERROR(Status);

  // Initialize Platform PreLoader HOBs
  if (PreEnv->Header == PRELOADER_HEADER) {
    Crc32         = PreEnv->Crc32;
    PreEnv->Crc32 = 0x0;
    if (CalculateCrc32(PreEnv, sizeof(PRELOADER_ENVIRONMENT)) == Crc32) {
      PreEnv->Crc32 = Crc32;
      DEBUG((EFI_D_INFO, "CRC32 check succeeded \n"));
    }
    else {
      // Hey we have memory corrpution
      DEBUG((EFI_D_ERROR, "CRC32 check failed \n"));
      ASSERT(FALSE);
    }
  }

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "Launching CPUs\n"));

  // Launch all CPUs
  if (ArmReadMpidr() == 0x80000000) {
    for (UINTN i = 1; i < 8; i++) {
      ARM_HVC_ARGS ArmHvcArgs;
      ArmHvcArgs.Arg0 = ARM_SMC_ID_PSCI_CPU_ON_AARCH64;
      ArmHvcArgs.Arg1 = ProcessorIdMapping[i];
      ArmHvcArgs.Arg2 = (UINTN)&SecondaryCpuEntry;
      ArmHvcArgs.Arg3 = i;

      ArmCallHvc(&ArmHvcArgs);
      ASSERT(ArmHvcArgs.Arg0 == ARM_SMC_PSCI_RET_SUCCESS);
    }
  }

  // Now, the HOB List has been initialized, we can register performance
  // information PERF_START (NULL, "PEI", NULL, StartTimeStamp);

  // SEC phase needs to run library constructors by hand.
  ProcessLibraryConstructorList();

  // Assume the FV that contains the PI (our code) also contains a
  // compressed FV.
  Status = DecompressFirstFv();
  ASSERT_EFI_ERROR(Status);

  // Load the DXE Core and transfer control to it
  Status = LoadDxeCoreFromFv(NULL, 0);
  ASSERT_EFI_ERROR(Status);

  // We should never reach here
  CpuDeadLoop();
}

VOID CEntryPoint(IN VOID *StackBase, IN UINTN StackSize)
{
  Main(StackBase, StackSize, 0);
}

extern void ResetFb();

VOID SecondaryCEntryPoint(IN UINTN Index)
{
  ASSERT(Index >= 1 && Index <= 7);

  EFI_PHYSICAL_ADDRESS MailboxAddress =
      FixedPcdGet64(SecondaryCpuMpParkRegionBase) + 0x10000 * Index + 0x1000;
  PEFI_PROCESSOR_MAILBOX pMailbox =
      (PEFI_PROCESSOR_MAILBOX)(VOID *)MailboxAddress;

  UINT32 CurrentProcessorId = 0;
  VOID (*SecondaryStart)(VOID * pMailbox);
  UINTN SecondaryEntryAddr;
  UINTN InterruptId;
  UINTN AcknowledgeInterrupt;

  // MMU, cache and branch predicton must be disabled
  // Cache is disabled in CRT startup code
  ArmDisableMmu();
  ArmDisableBranchPrediction();

  // Turn on GIC CPU interface as well as SGI interrupts
  ArmGicEnableInterruptInterface(FixedPcdGet64(PcdGicInterruptInterfaceBase));
  MmioWrite32(FixedPcdGet64(PcdGicInterruptInterfaceBase) + 0x4, 0xf0);

  // But turn off interrupts
  ArmDisableInterrupts();

  // Clear mailbox
  pMailbox->JumpAddress = 0;
  pMailbox->ProcessorId = 0xffffffff;
  CurrentProcessorId    = ProcessorIdMapping[Index];

  do {
    // ArmDataSynchronizationBarrier();
    // DEBUG((EFI_D_ERROR, "%d: WFI \n", Index));
    // ArmCallWFI();
    // DEBUG((EFI_D_ERROR, "%d: end WFI \n", Index));
    ArmDataSynchronizationBarrier();

    if (pMailbox->ProcessorId == Index) {
      SecondaryEntryAddr = pMailbox->JumpAddress;
    }

    AcknowledgeInterrupt = ArmGicAcknowledgeInterrupt(
        FixedPcdGet64(PcdGicInterruptInterfaceBase), &InterruptId);
    if (InterruptId <
        ArmGicGetMaxNumInterrupts(FixedPcdGet64(PcdGicDistributorBase))) {
      // Got a valid SGI number hence signal End of Interrupt
      ArmGicEndOfInterrupt(
          FixedPcdGet64(PcdGicInterruptInterfaceBase), AcknowledgeInterrupt);
    }
  } while (SecondaryEntryAddr == 0);

  // Acknowledge this one
  pMailbox->JumpAddress = 0;

  SecondaryStart = (VOID(*)())SecondaryEntryAddr;
  SecondaryStart(pMailbox);

  // Should never reach here
  ASSERT(FALSE);
}
