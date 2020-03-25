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
extern void _ModuleEntryPoint();

static UINT32 ProcessorIdMapping[8] = {
    0x00000000, 0x00000001, 0x00000002, 0x00000003,
    0x00000100, 0x00000101, 0x00000102, 0x00000103,
};

// The EL2 shellcode is pre-compiled. For detailed explanation, see
// El2Redirection.S in the same directory
UINT8 El2ShellCode[120] = {
    0xDF, 0x43, 0x03, 0xD5, 0xDF, 0x3F, 0x03, 0xD5, 0x41, 0x42, 0x38, 0xD5,
    0x3F, 0x20, 0x00, 0xF1, 0x0C, 0x01, 0x00, 0x54, 0xA0, 0x00, 0x00, 0x54,
    0x41, 0x00, 0x00, 0xB5, 0x00, 0x00, 0x00, 0x14, 0x00, 0x10, 0x38, 0xD5,
    0x04, 0x00, 0x00, 0x14, 0x00, 0x10, 0x3C, 0xD5, 0x02, 0x00, 0x00, 0x14,
    0x00, 0x10, 0x3E, 0xD5, 0xA1, 0x00, 0x82, 0x92, 0x00, 0x00, 0x01, 0x8A,
    0x41, 0x42, 0x38, 0xD5, 0x3F, 0x20, 0x00, 0xF1, 0x0C, 0x01, 0x00, 0x54,
    0xA0, 0x00, 0x00, 0x54, 0x41, 0x00, 0x00, 0xB5, 0x00, 0x00, 0x00, 0x14,
    0x00, 0x10, 0x18, 0xD5, 0x04, 0x00, 0x00, 0x14, 0x00, 0x10, 0x1C, 0xD5,
    0x02, 0x00, 0x00, 0x14, 0x00, 0x10, 0x1E, 0xD5, 0x9F, 0x3F, 0x03, 0xD5,
    0xDF, 0x3F, 0x03, 0xD5, 0x00, 0x04, 0xA0, 0xD2, 0x00, 0x00, 0x1F, 0xD6,
};

STATIC VOID UartInit(VOID)
{
  SerialPortInitialize();

  DEBUG((EFI_D_INFO, "\nTianoCore on 950XL (AArch64)\n"));
  DEBUG(
      (EFI_D_INFO, "Firmware version %s built %a %a\n\n",
       (CHAR16 *)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__));
}

VOID WaitForSecondaryCPUs(VOID)
{
  while (1) {

    BOOLEAN IsAllCpuLaunched = TRUE;

    for (UINTN Index = 1; Index < 7; Index++) {
      EFI_PHYSICAL_ADDRESS MailboxAddress =
          FixedPcdGet64(SecondaryCpuMpParkRegionBase) + 0x10000 * Index +
          0x1000;
      PEFI_PROCESSOR_MAILBOX pMailbox =
          (PEFI_PROCESSOR_MAILBOX)(VOID *)MailboxAddress;

      ArmDataSynchronizationBarrier();
      if (pMailbox->El2JumpFlag != 0xfffffff1) {
        IsAllCpuLaunched = FALSE;
        break;
      }
    }

    // All CPU started. Continue.
    if (IsAllCpuLaunched) {
      break;
    }
  }
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

  // Initialize (fake) UART.
  UartInit();

  // Which EL?
  if (ArmReadCurrentEL() == AARCH64_EL2) {
    DEBUG((EFI_D_ERROR, "Running at EL2 \n"));
  }
  else {
    DEBUG((EFI_D_ERROR, "Running at EL1 \n"));
  }

  // Immediately launch all CPUs, 7 CPUs hold
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "Launching CPUs\n"));

  // Launch all CPUs, hold and jump to EL2 (only for EL1)
  if (ArmReadCurrentEL() == AARCH64_EL1) {
#if 0
    if (ArmReadMpidr() == 0x80000000) {
      for (UINTN i = 1; i < 7; i++) {
        ARM_HVC_ARGS ArmHvcArgs;
        ArmHvcArgs.Arg0 = ARM_SMC_ID_PSCI_CPU_ON_AARCH64;
        ArmHvcArgs.Arg1 = ProcessorIdMapping[i];
        ArmHvcArgs.Arg2 = (UINTN)&_ModuleEntryPoint;
        ArmHvcArgs.Arg3 = i;

        ArmCallHvc(&ArmHvcArgs);
        ASSERT(ArmHvcArgs.Arg0 == ARM_SMC_PSCI_RET_SUCCESS);
      }
    }

    DEBUG((EFI_D_ERROR, "Waiting for all CPUs...\n"));
    WaitForSecondaryCPUs();
    DEBUG((EFI_D_ERROR, "All CPU started.\n"));
#endif
    DEBUG((EFI_D_ERROR, "Injecting shellcode...\n"));
    EFI_PHYSICAL_ADDRESS El2ExceptionHandlerAddr = 0x06c01984;
    UINT8 *El2ExceptionHandler = (UINT8 *)(VOID *)El2ExceptionHandlerAddr;
    CopyMem(El2ExceptionHandler, El2ShellCode, sizeof(El2ShellCode));
    ArmDataSynchronizationBarrier();
    DEBUG((EFI_D_ERROR, "You have been served\n"));

    // Jump overself
    ARM_HVC_ARGS StubArg;
    ArmCallHvc(&StubArg);
    // We should not reach here
    CpuDeadLoop();
  }
  else if (ArmReadCurrentEL() == AARCH64_EL2) {
#if 0
    // Looks good. Notify all secondary CPUs to jump!
    for (UINTN Index = 1; Index < 7; Index++) {
      EFI_PHYSICAL_ADDRESS MailboxAddress =
          FixedPcdGet64(SecondaryCpuMpParkRegionBase) + 0x10000 * Index +
          0x1000;
      PEFI_PROCESSOR_MAILBOX pMailbox =
          (PEFI_PROCESSOR_MAILBOX)(VOID *)MailboxAddress;

      pMailbox->El2JumpFlag = 0xfffffff2;
      ArmDataSynchronizationBarrier();
    }

    // Make sure they are all initialized
    DEBUG((EFI_D_ERROR, "Waiting for all CPUs...\n"));
    WaitForSecondaryCPUs();
    DEBUG((EFI_D_ERROR, "All CPU started.\n"));
#endif
  }
  else {
    // How do I get there?
    ASSERT(FALSE);
  }

  // Architecture-specific initialization
  // Enable cache
  ArmInvalidateDataCache();
  ArmEnableInstructionCache();
  ArmEnableDataCache();

  // Enable Floating Point
  ArmEnableVFP();

  /* Enable program flow prediction, if supported */
  ArmEnableBranchPrediction();

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

  // Clear mailbox
  pMailbox->JumpAddress = 0;
  pMailbox->ProcessorId = 0xffffffff;
  // Notify the main processor that we are here
  pMailbox->El2JumpFlag = 0xfffffff1;
  CurrentProcessorId    = ProcessorIdMapping[Index];
  ArmDataSynchronizationBarrier();

  // Check Exception Level and hold if we are in EL1
  if (ArmReadCurrentEL() == AARCH64_EL1) {
    while (1) {
      ArmDataSynchronizationBarrier();
      if (pMailbox->El2JumpFlag == 0xfffffff2) {
        /* Acknowledge and jump */
        pMailbox->El2JumpFlag = 0xfffffff3;
        ArmDataSynchronizationBarrier();

        ARM_HVC_ARGS StubArgs;
        ArmCallHvc(&StubArgs);
        /* We should not reach here */
        CpuDeadLoop();
      }
    }
  }

  // Turn on GIC CPU interface as well as SGI interrupts
  ArmGicEnableInterruptInterface(FixedPcdGet64(PcdGicInterruptInterfaceBase));
  MmioWrite32(FixedPcdGet64(PcdGicInterruptInterfaceBase) + 0x4, 0xf0);

  // But turn off interrupts
  ArmDisableInterrupts();

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
