#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Library/ArmSmcLib.h>
#include <Library/IoLib.h>
#include <Library/LKEnvLib.h>
#include <Library/QcomSmcLib.h>

#include <Chipset/scm.h>

STATIC
EFIAPI
UINTN
SmInternalCall(UINTN CommandAddr)
{

  ARM_SMC_ARGS ArmSmcArgs;
  int          ContextId;

  ArmSmcArgs.Arg0 = 1;
  ArmSmcArgs.Arg1 = (uintptr_t)&ContextId;
  ArmSmcArgs.Arg2 = CommandAddr;

  do {
    ArmCallSmc(&ArmSmcArgs);
  } while (ArmSmcArgs.Arg0 == SCM_INTERRUPTED);

  return ArmSmcArgs.Arg0;
}

STATIC
EFIAPI
EFI_STATUS
SmInternalCallAtomic(uint32_t Service, uint32_t Command, uint32_t Arg1)
{
  ARM_SMC_ARGS ArmSmcArgs;
  UINTN        ContextId;

  ArmSmcArgs.Arg0 = SCM_ATOMIC(Service, Command, Arg1);
  ArmSmcArgs.Arg1 = (uintptr_t)&ContextId;
  ArmSmcArgs.Arg2 = arg1;

  ArmCallSmc(&ArmSmcArgs);

  return SmRemapError(ArmSmcArgs.Arg0);
}

STATIC
EFIAPI
EFI_STATUS
SmInternalCallCommand(const struct scm_command *cmd)
{
  int ScmReturn = 0;

  /*
   * Flush the command buffer so that the secure world sees
   * the correct data.
   */
  WriteBackDataCacheRange((VOID *)cmd, cmd->len);

  ScmReturn = SmInternalCall((uintptr_t)(VOID *)cmd);
  return SmRemapError(ScmReturn);
}

STATIC
EFIAPI
EFI_STATUS
SmRemapError(int ScmError)
{
  if (ScmError != SCM_EBUSY) {
    DEBUG((EFI_D_ERROR, "SCM call failed with error code %d\n", ScmError));
  }

  switch (ScmError) {
  case SCM_ERROR:
    return EFI_ABORTED;
  case SCM_EINVAL_ADDR:
  case SCM_EINVAL_ARG:
    return EFI_INVALID_PARAMETER;
  case SCM_EOPNOTSUPP:
    return EFI_UNSUPPORTED;
  case SCM_ENOMEM:
    return EFI_OUT_OF_RESOURCES;
  case SCM_EBUSY:
    return EFI_ALREADY_STARTED;
  case SCM_SUCCESS:
    return EFI_SUCCESS;
  }

  return EFI_INVALID_PARAMETER;
}

STATIC
EFIAPI
EFI_STATUS
SmCallCommon(
    UINT32 svc_id, UINT32 cmd_id, const void *cmd_buf, size_t cmd_len,
    void *resp_buf, size_t resp_len, struct scm_command *scm_buf,
    size_t scm_buf_length)
{
}