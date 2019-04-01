#ifndef __QCOM_PROTOCOL_I2C_QUP_H__
#define __QCOM_PROTOCOL_I2C_QUP_H__

#include <Chipset/i2c_qup.h>

#define QCOM_I2C_QUP_PROTOCOL_GUID                                             \
  {                                                                            \
    0x2c898318, 0x41c1, 0x4309,                                                \
    {                                                                          \
      0x89, 0x8a, 0x2f, 0x55, 0xc8, 0xcf, 0x0b, 0x86                           \
    }                                                                          \
  }

typedef struct _QCOM_I2C_QUP_PROTOCOL QCOM_I2C_QUP_PROTOCOL;
typedef VOID(EFIAPI *qup_i2c_iterate_cb_t)(UINT8 id);

typedef struct qup_i2c_dev *(EFIAPI *qup_i2c_get_dev_t)(UINTN id);
typedef int(EFIAPI *qup_i2c_xfer_t)(
    struct qup_i2c_dev *dev, struct i2c_msg msgs[], int num);
typedef void(EFIAPI *qup_i2c_iterate_t)(qup_i2c_iterate_cb_t cb);

struct _QCOM_I2C_QUP_PROTOCOL {
  qup_i2c_get_dev_t GetDevice;
  qup_i2c_xfer_t    Transfer;
  qup_i2c_iterate_t Iterate;
};

extern EFI_GUID gQcomI2cQupProtocolGuid;

#endif
