#include <PiPei.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/PlatformPrePiLib.h>
#include "PlatformUtils.h"

#include <IndustryStandard/ArmStdSmc.h>
#include <Library/ArmHvcLib.h>
#include <Library/ArmSmcLib.h>

#include "HvcPatch.h"

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

BOOLEAN IsLinuxBootRequested(VOID)
{
  return FALSE;
}

VOID PlatformInitialize(VOID)
{
  PsciFixupInit();
  QGicPeim();
}