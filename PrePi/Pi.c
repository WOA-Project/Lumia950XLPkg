// Pi.c: Entry point for SEC(Security).

#include "Pi.h"
#include "HvcPatch.h"

#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <PiDxe.h>
#include <PiPei.h>

#include <Configuration/Hob.h>
#include <Guid/LzmaDecompress.h>
#include <Ppi/GuidedSectionExtraction.h>

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

#include <IndustryStandard/ArmStdSmc.h>
#include <Library/ArmHvcLib.h>
#include <Library/ArmSmcLib.h>

#include "El2Redirection.h"

VOID EFIAPI ProcessLibraryConstructorList(VOID);

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

VOID WaitForSecondaryCPUs(VOID)
{
  while (1) {

    BOOLEAN IsAllCpuLaunched = TRUE;

    for (UINTN Index = 1; Index < FixedPcdGet32(PcdCoreCount); Index++) {
      EFI_PHYSICAL_ADDRESS MailboxAddress =
          FixedPcdGet64(SecondaryCpuMpParkRegionBase) + 0x10000 * Index +
          0x1000;
      PEFI_PROCESSOR_MAILBOX pMailbox =
          (PEFI_PROCESSOR_MAILBOX)(VOID *)MailboxAddress;

      ArmDataSynchronizationBarrier();
      if (pMailbox->El2JumpFlag != EL2REDIR_MAILBOX_READY) {
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

VOID InstallEl2Patch(VOID)
{
  // The big EL2 HVC call handler is located at 0x06c01984,
  // but if secondary CPUs are launched, exception vector will be
  // fixed so CPU0 call would fail. Therefore we patched
  // PSCI_CPU_SUSPEND_AARCH64 handler at 0x06c03aa8.
  DEBUG((EFI_D_ERROR, "Injecting shellcode...\n"));
  EFI_PHYSICAL_ADDRESS PsciCpuSuspendHandlerAddr = FixedPcdGet64(PsciCpuSuspendAddress);
  UINT8 *PsciCpuSuspendHandler = (UINT8 *)(VOID *)PsciCpuSuspendHandlerAddr;
  CopyMem(PsciCpuSuspendHandler, El2ShellCode, sizeof(El2ShellCode));
  ArmDataSynchronizationBarrier();
  ArmInvalidateDataCache();
  ArmInvalidateInstructionCache();
  DEBUG((EFI_D_ERROR, "You have been served\n"));
}

STATIC VOID MpParkEl2Init(VOID)
{
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
    if (ArmReadMpidr() == 0x80000000) {
      for (UINTN i = 1; i < FixedPcdGet32(PcdCoreCount); i++) {
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

    // Install patch
    InstallEl2Patch();

    // Looks good. Notify all secondary CPUs to jump!
    for (UINTN Index = 1; Index < FixedPcdGet32(PcdCoreCount); Index++) {
      EFI_PHYSICAL_ADDRESS MailboxAddress =
          FixedPcdGet64(SecondaryCpuMpParkRegionBase) + 0x10000 * Index +
          0x1000;
      PEFI_PROCESSOR_MAILBOX pMailbox =
          (PEFI_PROCESSOR_MAILBOX)(VOID *)MailboxAddress;

      pMailbox->El2JumpFlag = EL2REDIR_MAILBOX_SIGNAL;
      ArmDataSynchronizationBarrier();
    }

    // Make sure they are all initialized
    DEBUG((EFI_D_ERROR, "Waiting for all CPUs...\n"));
    WaitForSecondaryCPUs();
    DEBUG((EFI_D_ERROR, "All CPU started.\n"));
    ArmDataSynchronizationBarrier();

    DEBUG((EFI_D_ERROR, "Jump CPU0 to EL2.\n"));
    ArmDataSynchronizationBarrier();

    // Install patch again
    InstallEl2Patch();

    // Jump overself
    ARM_HVC_ARGS StubArg;
    StubArg.Arg0 = ARM_SMC_ID_PSCI_CPU_SUSPEND_AARCH64;
    ArmCallHvc(&StubArg);

    // We should not reach here
    ASSERT(FALSE);
  }
}

STATIC VOID MpParkNotifySecondaryCPUs(VOID)
{
  // Notify secondary CPUs that GIC is set up
  for (UINTN Index = 1; Index < FixedPcdGet32(PcdCoreCount); Index++) {
    EFI_PHYSICAL_ADDRESS MailboxAddress =
        FixedPcdGet64(SecondaryCpuMpParkRegionBase) + 0x10000 * Index + 0x1000;
    PEFI_PROCESSOR_MAILBOX pMailbox =
        (PEFI_PROCESSOR_MAILBOX)(VOID *)MailboxAddress;

    pMailbox->El2JumpFlag = EL2REDIR_MAILBOX_SIGNAL;
    ArmDataSynchronizationBarrier();
  }

  DEBUG((EFI_D_ERROR, "Waiting for all CPUs...\n"));
  WaitForSecondaryCPUs();
  DEBUG((EFI_D_ERROR, "All CPU entered MpPark.\n"));
  ArmDataSynchronizationBarrier();
}

STATIC VOID MpParkEl1Init(VOID)
{
  // Immediately launch all CPUs, 7 CPUs hold
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "Launching CPUs\n"));

  if (ArmReadMpidr() == 0x80000000) {
    for (UINTN i = 1; i < FixedPcdGet32(PcdCoreCount); i++) {
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

  // Looks good. Notify all secondary CPUs to jump!
  for (UINTN Index = 1; Index < FixedPcdGet32(PcdCoreCount); Index++) {
    EFI_PHYSICAL_ADDRESS MailboxAddress =
        FixedPcdGet64(SecondaryCpuMpParkRegionBase) + 0x10000 * Index +
        0x1000;
    PEFI_PROCESSOR_MAILBOX pMailbox =
        (PEFI_PROCESSOR_MAILBOX)(VOID *)MailboxAddress;

    pMailbox->El2JumpFlag = EL2REDIR_MAILBOX_STAY_EL1;
    ArmDataSynchronizationBarrier();
  }

  // Make sure they are all initialized
  DEBUG((EFI_D_ERROR, "Waiting for all CPUs...\n"));
  WaitForSecondaryCPUs();
  DEBUG((EFI_D_ERROR, "All CPU started.\n"));
  ArmDataSynchronizationBarrier();
}

STATIC VOID PsciFixupInit(VOID)
{
  EFI_PHYSICAL_ADDRESS WakeFromPowerGatePatchOffset;
  EFI_PHYSICAL_ADDRESS LowerELSynchronous64PatchOffset;
  EFI_PHYSICAL_ADDRESS LowerELSynchronous32PatchOffset;
  ARM_HVC_ARGS         StubArgsHvc;
  ARM_SMC_ARGS         StubArgsSmc;

  WakeFromPowerGatePatchOffset    = WAKE_FROM_POWERGATE_PATCH_ADDR;
  LowerELSynchronous64PatchOffset = LOWER_EL_SYNC_EXC_64B_PATCH_ADDR;
  LowerELSynchronous32PatchOffset = LOWER_EL_SYNC_EXC_32B_PATCH_ADDR;

  CopyMem(
      (VOID *)WakeFromPowerGatePatchOffset, WakeFromPowerGatePatchHandler,
      sizeof(WakeFromPowerGatePatchHandler));
  CopyMem(
      (VOID *)LowerELSynchronous64PatchOffset, LowerELSynchronous64PatchHandler,
      sizeof(LowerELSynchronous64PatchHandler));
  CopyMem(
      (VOID *)LowerELSynchronous32PatchOffset, LowerELSynchronous32PatchHandler,
      sizeof(LowerELSynchronous32PatchHandler));

  ArmDataSynchronizationBarrier();
  ArmInvalidateDataCache();
  ArmInvalidateInstructionCache();

  // Call into the handler to make HCR_EL2.TSC sticky
  StubArgsHvc.Arg0 = ARM_SMC_ID_PSCI_VERSION;
  ArmCallHvc(&StubArgsHvc);

  // Well...
  StubArgsSmc.Arg0 = ARM_SMC_ID_PSCI_VERSION;
  ArmCallSmc(&StubArgsSmc);
}

VOID Main(IN VOID *StackBase, IN UINTN StackSize, IN UINT64 StartTimeStamp)
{

  EFI_HOB_HANDOFF_INFO_TABLE *     HobList;
  PRELOADER_ENVIRONMENT_VERSION_2 *PreEnv = (VOID *)PRELOADER_ENV_ADDR;
  UINT32                           Crc32  = 0;
  EFI_STATUS                       Status;

  UINTN MemoryBase     = 0;
  UINTN MemorySize     = 0;
  UINTN UefiMemoryBase = 0;
  UINTN UefiMemorySize = 0;

  UINTN BootMode                 = BOOT_MODE_PSCI;
  UINTN EnablePlatformSdCardBoot = 0;
  UINTN UseQuadCoreConfiguration = 0;

  // Initialize (fake) UART.
  UartInit();

  // Initialize Platform PreLoader HOBs
  if (PreEnv->Header == PRELOADER_HEADER) {
    Crc32         = PreEnv->Crc32;
    PreEnv->Crc32 = 0x0;
    if (CalculateCrc32(PreEnv, sizeof(PRELOADER_ENVIRONMENT_VERSION_1)) == Crc32) {
      PreEnv->Crc32 = Crc32;
      DEBUG((EFI_D_INFO, "CRC32 check succeeded \n"));
    }
    else {
      // Hey we have memory corrpution
      DEBUG((EFI_D_ERROR, "CRC32 check failed \n"));
      ASSERT(FALSE);
    }

    if (PreEnv->PreloaderVersion >= PRELOADER_VERSION_MIN) {
      Crc32           = PreEnv->Crc32v2;
      PreEnv->Crc32v2 = 0x0;
      if (CalculateCrc32(PreEnv, sizeof(PRELOADER_ENVIRONMENT_VERSION_2)) == Crc32) {
        PreEnv->Crc32v2 = Crc32;
        DEBUG((EFI_D_INFO, "CRC32v2 check succeeded \n"));
      }
      else {
        // Hey we have memory corrpution
        DEBUG((EFI_D_ERROR, "CRC32v2 check failed \n"));
        ASSERT(FALSE);
      }

      BootMode                 = PreEnv->BootMode;
      EnablePlatformSdCardBoot = PreEnv->EnablePlatformSdCardBoot;
      UseQuadCoreConfiguration = PreEnv->UseQuadCoreConfiguration;
    }
  }

  switch (BootMode) {
    case BOOT_MODE_PSCI:
      // PSCI fixup init
      PsciFixupInit();
      break;
    case BOOT_MODE_MPPARK:
      // EL1 init
      MpParkEl1Init();
      break;
    case BOOT_MODE_MPPARK_EL2:
      // EL2 init
      MpParkEl2Init();
      break;
  }

  // Architecture-specific initialization
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

  switch (BootMode) {
    case BOOT_MODE_MPPARK:
    case BOOT_MODE_MPPARK_EL2:
      // Notify secondary CPUs that GIC is setup up
      MpParkNotifySecondaryCPUs();
      break;
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

  // Now, the HOB List has been initialized, we can register performance
  // information PERF_START (NULL, "PEI", NULL, StartTimeStamp);

  // SEC phase needs to run library constructors by hand.
  ProcessLibraryConstructorList();

  // Assume the FV that contains the PI (our code) also contains a compressed
  // FV.
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

  CurrentProcessorId    = ProcessorIdMapping[Index];

  // Notify the main processor that we are here
  pMailbox->El2JumpFlag = EL2REDIR_MAILBOX_READY;
  ArmDataSynchronizationBarrier();

  // Check Exception Level and hold if we are in EL1
  if (ArmReadCurrentEL() == AARCH64_EL1) {
    while (1) {
      ArmDataSynchronizationBarrier();
      if (pMailbox->El2JumpFlag == EL2REDIR_MAILBOX_SIGNAL) {
        /* Acknowledge and jump */
        pMailbox->El2JumpFlag = EL2REDIR_MAILBOX_ACKNOWLEDGE;
        ArmDataSynchronizationBarrier();

        ARM_HVC_ARGS StubArg;
        StubArg.Arg0 = ARM_SMC_ID_PSCI_CPU_SUSPEND_AARCH64;
        ArmCallHvc(&StubArg);
        /* We should not reach here */
        CpuDeadLoop();
      }
      else if (pMailbox->El2JumpFlag == EL2REDIR_MAILBOX_STAY_EL1) {
        // Hold at EL1, until CPU0 tell us that GIC is set up
        break;
      }
    }

    ArmDataSynchronizationBarrier();
    if (pMailbox->El2JumpFlag == EL2REDIR_MAILBOX_STAY_EL1) {
      // Notify the main processor that we are here
      pMailbox->El2JumpFlag = EL2REDIR_MAILBOX_READY;
      ArmDataSynchronizationBarrier();

      // Hold at EL1, until CPU0 tell us that GIC is set up
      while (1) {
        ArmDataSynchronizationBarrier();
        if (pMailbox->El2JumpFlag == EL2REDIR_MAILBOX_SIGNAL) {
          /* Just set to ready and continue setup */
          pMailbox->El2JumpFlag = EL2REDIR_MAILBOX_READY;
          ArmDataSynchronizationBarrier();
          break;
        }
      }
    }
  }
  else {
    // Hold at EL2, until CPU0 tell us that GIC is set up
    while (1) {
      ArmDataSynchronizationBarrier();
      if (pMailbox->El2JumpFlag == EL2REDIR_MAILBOX_SIGNAL) {
        /* Just set to ready and continue setup */
        pMailbox->El2JumpFlag = EL2REDIR_MAILBOX_READY;
        ArmDataSynchronizationBarrier();
        break;
      }
    }
  }

  // Turn on GIC CPU interface as well as SGI interrupts
  ArmGicEnableInterruptInterface(FixedPcdGet64(PcdGicInterruptInterfaceBase));
  MmioWrite32(FixedPcdGet64(PcdGicInterruptInterfaceBase) + 0x4, 0xf0);

  // But turn off interrupts
  ArmDisableInterrupts();

  do {
    // Technically we should do a WFI
    // But we just spin here instead
    ArmDataSynchronizationBarrier();

    // Technically the CPU ID should be checked
    // against request per MpPark spec,
    // but the actual Windows implementation guarantees
    // that no CPU will be started simultaneously,
    // so the check was made optional.
    //
    // This also enables "spin-table" startup method
    // for Linux.
    //
    // Example usage:
    // enable-method = "spin-table";
    // cpu-release-addr = <0 0x00311008>;
    if (FixedPcdGetBool(SecondaryCpuIgnoreCpuIdCheck) ||
        pMailbox->ProcessorId == Index) {
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