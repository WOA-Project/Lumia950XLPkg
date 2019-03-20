#ifndef __QCOM_PROTOCOL_RPM_H__
#define __QCOM_PROTOCOL_RPM_H__

#define QCOM_RPM_PROTOCOL_GUID                                                 \
  {                                                                            \
    0x10c51490, 0xd3b6, 0x45de,                                                \
    {                                                                          \
      0xb6, 0xfc, 0x8f, 0x46, 0xa2, 0xeb, 0x8e, 0x01                           \
    }                                                                          \
  }

typedef struct _QCOM_RPM_PROTOCOL QCOM_RPM_PROTOCOL;

typedef enum {
  RPM_REQUEST_TYPE,
  RPM_CMD_TYPE,
  RPM_SUCCESS_REQ_ACK,
  RPM_SUCCESS_CMD_ACK,
  RPM_ERROR_ACK,
} msg_type;

/* RPM regulator request types */
enum rpm_regulator_type {
  RPM_REGULATOR_TYPE_LDO,
  RPM_REGULATOR_TYPE_SMPS,
  RPM_REGULATOR_TYPE_VS,
  RPM_REGULATOR_TYPE_NCP,
  RPM_REGULATOR_TYPE_MAX,
};

/* RPM resource parameters */
enum rpm_regulator_param_index {
  RPM_REGULATOR_PARAM_ENABLE,
  RPM_REGULATOR_PARAM_VOLTAGE,
  RPM_REGULATOR_PARAM_CURRENT,
  RPM_REGULATOR_PARAM_MODE_LDO,
  RPM_REGULATOR_PARAM_MODE_SMPS,
  RPM_REGULATOR_PARAM_PIN_CTRL_ENABLE,
  RPM_REGULATOR_PARAM_PIN_CTRL_MODE,
  RPM_REGULATOR_PARAM_FREQUENCY,
  RPM_REGULATOR_PARAM_HEAD_ROOM,
  RPM_REGULATOR_PARAM_QUIET_MODE,
  RPM_REGULATOR_PARAM_FREQ_REASON,
  RPM_REGULATOR_PARAM_CORNER,
  RPM_REGULATOR_PARAM_BYPASS,
  RPM_REGULATOR_PARAM_FLOOR_CORNER,
  RPM_REGULATOR_PARAM_MAX,
};

enum rpm_regulator_smps_mode {
  RPM_REGULATOR_SMPS_MODE_AUTO  = 0,
  RPM_REGULATOR_SMPS_MODE_IPEAK = 1,
  RPM_REGULATOR_SMPS_MODE_PWM   = 2,
};

enum rpm_regulator_ldo_mode {
  RPM_REGULATOR_LDO_MODE_IPEAK = 0,
  RPM_REGULATOR_LDO_MODE_HPM   = 1,
};

typedef INTN(EFIAPI *rpm_send_data_t)(UINT32 *data, UINT32 len, msg_type type);
typedef VOID(EFIAPI *rpm_clk_enable_t)(UINT32 *data, UINT32 len);
typedef EFI_STATUS(EFIAPI *rpm_ldo_enable_t)();

struct _QCOM_RPM_PROTOCOL {
  rpm_send_data_t  rpm_send_data;
  rpm_clk_enable_t rpm_clk_enable;
  rpm_ldo_enable_t rpm_ldo_pipe_enable;
  rpm_ldo_enable_t rpm_ldo30_enable;
};

extern EFI_GUID gQcomRpmProtocolGuid;

#endif
