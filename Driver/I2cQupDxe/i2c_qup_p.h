#ifndef _I2C_QUP_PRIVATE_H
#define _I2C_QUP_PRIVATE_H

/* Function Definitions */
void qup_i2c_sec_init(
    struct qup_i2c_dev *dev, uint32_t clk_freq, uint32_t src_clk_rate);
int qup_i2c_deinit(struct qup_i2c_dev *dev);
int qup_i2c_xfer(struct qup_i2c_dev *dev, struct i2c_msg msgs[], int num);

#endif
