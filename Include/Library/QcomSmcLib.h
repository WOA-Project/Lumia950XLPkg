#ifndef _QCOM_SMC_LIB_H_
#define _QCOM_SMC_LIB_H_

STATIC
EFIAPI
UINTN
SmInternalCall(UINTN CommandAddr);

STATIC
EFIAPI
EFI_STATUS
SmInternalCallAtomic(uint32_t Service, uint32_t Command, uint32_t Arg1);

STATIC
EFIAPI
EFI_STATUS
SmInternalCallCommand(const struct scm_command *cmd);

STATIC
EFIAPI
EFI_STATUS
SmRemapError(int ScmError);

#endif