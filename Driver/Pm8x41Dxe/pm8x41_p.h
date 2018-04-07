#ifndef _PM8x41_PRIVATE_H
#define _PM8x41_PRIVATE_H

#include <Device/pm8x41.h>
#include <Device/pm8x41_wled.h>

void pm8x41_reg_write(uint32_t addr, uint8_t val);
uint8_t pm8x41_reg_read(uint32_t addr);

void pm8x41_lpg_write(uint8_t chan, uint8_t off, uint8_t val);
void pm8x41_lpg_write_sid(uint8_t sid, uint8_t chan, uint8_t off, uint8_t val);
int pm8x41_gpio_get(uint8_t gpio, uint8_t *status);
int pm8x41_gpio_get_sid(uint8_t sid, uint8_t gpio, uint8_t *status);
int pm8x41_gpio_set(uint8_t gpio, uint8_t value);
int pm8x41_gpio_set_sid(uint8_t sid, uint8_t gpio, uint8_t value);
int pm8x41_gpio_config(uint8_t gpio, struct pm8x41_gpio *config);
int pm8x41_gpio_config_sid(uint8_t sid, uint8_t gpio, struct pm8x41_gpio *config);
void pm8x41_set_boot_done(void);
uint32_t pm8x41_v2_resin_status(void);
uint32_t pm8x41_resin_status(void);
void pm8x41_reset_configure(uint8_t);
void pm8994_reset_configure(uint8_t);
void pm8x41_v2_reset_configure(uint8_t);
int pm8x41_ldo_set_voltage(struct pm8x41_ldo *ldo, uint32_t voltage);
int pm8x41_ldo_control(struct pm8x41_ldo *ldo, uint8_t enable);
uint8_t pm8x41_get_pmic_rev(void);
uint8_t pm8x41_get_pon_reason(void);
uint8_t pm8x41_get_pon_poff_reason1(void);
uint8_t pm8x41_get_pon_poff_reason2(void);
uint32_t pm8x41_get_pwrkey_is_pressed(void);
void pm8x41_config_output_mpp(struct pm8x41_mpp *mpp);
void pm8x41_enable_mpp(struct pm8x41_mpp *mpp, enum mpp_en_ctl enable);
void pm8x41_enable_mvs(struct pm8x41_mvs *mvs, enum mvs_en_ctl enable);
uint8_t pm8x41_get_is_cold_boot(void);
void pm8x41_diff_clock_ctrl(uint8_t enable);
void pm8x41_clear_pmic_watchdog(void);
void pmi8994_config_mpp_slave_id(uint8_t slave_id);
void pm_pwm_enable(bool enable);
int pm_pwm_config(unsigned int duty_us, unsigned int period_us);

/* Function declations */
uint32_t pm8x41_adc_channel_read(uint16_t ch_num);
int pm8x41_iusb_max_config(uint32_t current);
int pm8x41_ibat_max_config(uint32_t current);
/* API: To set VDD max */
int pm8x41_chgr_vdd_max_config(uint32_t voltage);
/* API: To enable charging */
int pm8x41_chgr_ctl_enable(uint8_t enable);
/* API: Get battery voltage*/
uint32_t pm8x41_get_batt_voltage(void);
/* API: Get Voltage based State of Charge */
uint32_t pm8x41_get_voltage_based_soc(uint32_t cutoff_vol, uint32_t vdd_max);
/* API: Set the MMP pin as ADC */
void pm8x41_enable_mpp_as_adc(uint16_t mpp_num);

int pm8xxx_is_battery_broken(void);

void pm8x41_wled_config(struct pm8x41_wled_data *wled_ctrl);
void pm8x41_wled_iled_sync_control(uint8_t enable);
void pm8x41_wled_sink_control(uint8_t enable);
void pm8x41_wled_led_mod_enable(uint8_t enable);
void pm8x41_wled_enable(uint8_t enable);
void pm8x41_wled_config_slave_id(uint8_t slave_id);
uint8_t pm8x41_wled_reg_read(uint32_t addr);
void pm8x41_wled_reg_write(uint32_t addr, uint8_t val);

int pm_pwm_config(unsigned int duty_us, unsigned int period_us);
void pm_pwm_enable(bool enable);

void pm_vib_turn_on(void);
void pm_vib_turn_off(void);

#endif // _PM8x41_PRIVATE_H
