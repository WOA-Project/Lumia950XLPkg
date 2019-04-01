#ifndef __RUNTIME_SPMI_LIBRARY_H__
#define __RUNTIME_SPMI_LIBRARY_H__

typedef struct _pmic_arb_cmd {
  UINT8 opcode;
  UINT8 priority;
  UINT8 slave_id;
  UINT8 address;
  UINT8 offset;
  UINT8 byte_cnt;
} RUNTIME_PMIC_ARB_CMD, *PRUNTIME_PMIC_ARB_CMD;

typedef struct _pmic_arb_param {
  UINT8 *buffer;
  UINT8  size;
} RUNTIME_PMIC_ARB_PARAM, *PRUNTIME_PMIC_ARB_PARAM;

EFI_STATUS
EFIAPI
RtSpmiInitialize(VOID);

VOID EFIAPI RtSpmiLookupChannelNumber(VOID);

UINTN
EFIAPI
RtSpmiPmicArbWriteCmd(PRUNTIME_PMIC_ARB_CMD cmd, PRUNTIME_PMIC_ARB_PARAM param);

#endif