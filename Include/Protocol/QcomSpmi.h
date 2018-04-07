#ifndef __QCOM_PROTOCOL_SPMI_H__
#define __QCOM_PROTOCOL_SPMI_H__

#define QCOM_SPMI_PROTOCOL_GUID \
  { 0xa95ee608, 0x52be, 0x46c9, { 0x9f, 0x78, 0x03, 0x86, 0x42, 0xdb, 0xd0, 0x7c } }

typedef struct _QCOM_SPMI_PROTOCOL   QCOM_SPMI_PROTOCOL;

struct pmic_arb_cmd {
  UINT8 opcode;
  UINT8 priority;
  UINT8 slave_id;
  UINT8 address;
  UINT8 offset;
  UINT8 byte_cnt;
};

struct pmic_arb_param {
  UINT8 *buffer;
  UINT8 size;
};

typedef UINTN (EFIAPI *QCOM_SPMI_RW_FUNCTION)(struct pmic_arb_cmd *cmd, struct pmic_arb_param *param);

struct _QCOM_SPMI_PROTOCOL {
  QCOM_SPMI_RW_FUNCTION pmic_arb_write_cmd;
  QCOM_SPMI_RW_FUNCTION pmic_arb_read_cmd;
};

extern EFI_GUID gQcomSpmiProtocolGuid;

#endif
