#ifndef __QCOM_PROTOCOL_RPM_H__
#define __QCOM_PROTOCOL_RPM_H__

#define QCOM_RPM_PROTOCOL_GUID \
  { 0x10c51490, 0xd3b6, 0x45de, { 0xb6, 0xfc, 0x8f, 0x46, 0xa2, 0xeb, 0x8e, 0x01 } }

typedef struct _QCOM_RPM_PROTOCOL   QCOM_RPM_PROTOCOL;

typedef enum
{
  RPM_REQUEST_TYPE,
  RPM_CMD_TYPE,
  RPM_SUCCESS_REQ_ACK,
  RPM_SUCCESS_CMD_ACK,
  RPM_ERROR_ACK,
} msg_type;

typedef INTN (EFIAPI *rpm_send_data_t)(UINT32 *data, UINT32 len, msg_type type);
typedef VOID (EFIAPI *rpm_clk_enable_t)(UINT32 *data, UINT32 len);

struct _QCOM_RPM_PROTOCOL {
  rpm_send_data_t    rpm_send_data;
  rpm_clk_enable_t   rpm_clk_enable;
};

extern EFI_GUID gQcomRpmProtocolGuid;

#endif
