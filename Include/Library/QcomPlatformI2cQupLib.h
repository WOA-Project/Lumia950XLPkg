#ifndef __LIBRARY_QCOM_PLATFORM_I2C_QUP_LIB_H__
#define __LIBRARY_QCOM_PLATFORM_I2C_QUP_LIB_H__

#include <Chipset/i2c_qup.h>

typedef EFI_STATUS(EFIAPI *REGISTER_I2C_DEVICE)(
    struct qup_i2c_dev *device, UINTN clk_freq, UINTN src_clk_rate);

EFI_STATUS
EFIAPI
LibQcomPlatformI2cQupAddBusses(REGISTER_I2C_DEVICE RegisterI2cDevice);

#endif
