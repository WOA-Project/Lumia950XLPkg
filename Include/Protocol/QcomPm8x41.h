#ifndef __QCOM_PROTOCOL_PM8X41_H__
#define __QCOM_PROTOCOL_PM8X41_H__

#include <Device/pm8x41.h>
#include <Device/pm8x41_wled.h>

#define QCOM_PM8X41_PROTOCOL_GUID \
  { 0xb6e811d5, 0x1dce, 0x4ccb, { 0xaf, 0x21, 0xe9, 0xf7, 0xef, 0x68, 0x60, 0x7b } }

typedef struct _QCOM_PM8X41_PROTOCOL   QCOM_PM8X41_PROTOCOL;

typedef void     (EFIAPI *pm8x41_lpg_write_t)(uint8_t chan, uint8_t off, uint8_t val);
typedef void     (EFIAPI *pm8x41_lpg_write_sid_t)(uint8_t sid, uint8_t chan, uint8_t off, uint8_t val);
typedef int      (EFIAPI *pm8x41_gpio_get_t)(uint8_t gpio, uint8_t *status);
typedef int      (EFIAPI *pm8x41_gpio_get_sid_t)(uint8_t sid, uint8_t gpio, uint8_t *status);
typedef int      (EFIAPI *pm8x41_gpio_set_t)(uint8_t gpio, uint8_t value);
typedef int      (EFIAPI *pm8x41_gpio_set_sid_t)(uint8_t sid, uint8_t gpio, uint8_t value);
typedef int      (EFIAPI *pm8x41_gpio_config_t)(uint8_t gpio, struct pm8x41_gpio *config);
typedef int      (EFIAPI *pm8x41_gpio_config_sid_t)(uint8_t sid, uint8_t gpio, struct pm8x41_gpio *config);
typedef void     (EFIAPI *pm8x41_set_boot_done_t)(void);
typedef uint32_t (EFIAPI *pm8x41_v2_resin_status_t)(void);
typedef uint32_t (EFIAPI *pm8x41_resin_status_t)(void);
typedef void     (EFIAPI *pm8x41_reset_configure_t)(uint8_t);
typedef void     (EFIAPI *pm8994_reset_configure_t)(uint8_t);
typedef void     (EFIAPI *pm8x41_v2_reset_configure_t)(uint8_t);
typedef int      (EFIAPI *pm8x41_ldo_set_voltage_t)(struct pm8x41_ldo *ldo, uint32_t voltage);
typedef int      (EFIAPI *pm8x41_ldo_control_t)(struct pm8x41_ldo *ldo, uint8_t enable);
typedef uint8_t  (EFIAPI *pm8x41_get_pmic_rev_t)(void);
typedef uint8_t  (EFIAPI *pm8x41_get_pon_reason_t)(void);
typedef uint8_t  (EFIAPI *pm8x41_get_pon_poff_reason1_t)(void);
typedef uint8_t  (EFIAPI *pm8x41_get_pon_poff_reason2_t)(void);
typedef uint32_t (EFIAPI *pm8x41_get_pwrkey_is_pressed_t)(void);
typedef void     (EFIAPI *pm8x41_config_output_mpp_t)(struct pm8x41_mpp *mpp);
typedef void     (EFIAPI *pm8x41_enable_mpp_t)(struct pm8x41_mpp *mpp, enum mpp_en_ctl enable);
typedef void     (EFIAPI *pm8x41_enable_mvs_t)(struct pm8x41_mvs *mvs, enum mvs_en_ctl enable);
typedef uint8_t  (EFIAPI *pm8x41_get_is_cold_boot_t)(void);
typedef void     (EFIAPI *pm8x41_diff_clock_ctrl_t)(uint8_t enable);
typedef void     (EFIAPI *pm8x41_clear_pmic_watchdog_t)(void);
typedef void     (EFIAPI *pmi8994_config_mpp_slave_id_t)(uint8_t slave_id);
typedef int      (EFIAPI *pm8xxx_is_battery_broken_t)(void);

typedef uint32_t (EFIAPI *pm8x41_adc_channel_read_t)(uint16_t ch_num);
typedef int      (EFIAPI *pm8x41_iusb_max_config_t)(uint32_t current);
typedef int      (EFIAPI *pm8x41_ibat_max_config_t)(uint32_t current);
/* API: To set VDD max */
typedef int      (EFIAPI *pm8x41_chgr_vdd_max_config_t)(uint32_t voltage);
/* API: To enable charging */
typedef int      (EFIAPI *pm8x41_chgr_ctl_enable_t)(uint8_t enable);
/* API: Get battery voltage*/
typedef uint32_t (EFIAPI *pm8x41_get_batt_voltage_t)(void);
/* API: Get Voltage based State of Charge */
typedef uint32_t (EFIAPI *pm8x41_get_voltage_based_soc_t)(uint32_t cutoff_vol, uint32_t vdd_max);
/* API: Set the MMP pin as ADC */
typedef void     (EFIAPI *pm8x41_enable_mpp_as_adc_t)(uint16_t mpp_num);

typedef void     (EFIAPI *pm8x41_reg_write_t)(uint32_t addr, uint8_t val);
typedef uint8_t  (EFIAPI *pm8x41_reg_read_t)(uint32_t addr);

typedef void     (EFIAPI *pm8x41_wled_config_t)(struct pm8x41_wled_data *wled_ctrl);
typedef void     (EFIAPI *pm8x41_wled_iled_sync_control_t)(uint8_t enable);
typedef void     (EFIAPI *pm8x41_wled_sink_control_t)(uint8_t enable);
typedef void     (EFIAPI *pm8x41_wled_led_mod_enable_t)(uint8_t enable);
typedef void     (EFIAPI *pm8x41_wled_enable_t)(uint8_t enable);
typedef void     (EFIAPI *pm8x41_wled_config_slave_id_t)(uint8_t slave_id);
typedef uint8_t  (EFIAPI *pm8x41_wled_reg_read_t)(uint32_t addr);
typedef void     (EFIAPI *pm8x41_wled_reg_write_t)(uint32_t addr, uint8_t val);

typedef void     (EFIAPI *pm_pwm_enable_t)(bool enable);
typedef int      (EFIAPI *pm_pwm_config_t)(unsigned int duty_us, unsigned int period_us);

typedef void     (EFIAPI *pm_vib_turn_on_t)(void);
typedef void     (EFIAPI *pm_vib_turn_off_t)(void);

struct _QCOM_PM8X41_PROTOCOL {
  pm8x41_lpg_write_t             pm8x41_lpg_write;
  pm8x41_lpg_write_sid_t         pm8x41_lpg_write_sid;
  pm8x41_gpio_get_t              pm8x41_gpio_get;
  pm8x41_gpio_get_sid_t          pm8x41_gpio_get_sid;
  pm8x41_gpio_set_t              pm8x41_gpio_set;
  pm8x41_gpio_set_sid_t          pm8x41_gpio_set_sid;
  pm8x41_gpio_config_t           pm8x41_gpio_config;
  pm8x41_gpio_config_sid_t       pm8x41_gpio_config_sid;
  pm8x41_set_boot_done_t         pm8x41_set_boot_done;
  pm8x41_v2_resin_status_t       pm8x41_v2_resin_status;
  pm8x41_resin_status_t          pm8x41_resin_status;
  pm8x41_reset_configure_t       pm8x41_reset_configure;
  pm8994_reset_configure_t       pm8994_reset_configure;
  pm8x41_v2_reset_configure_t    pm8x41_v2_reset_configure;
  pm8x41_ldo_set_voltage_t       pm8x41_ldo_set_voltage;
  pm8x41_ldo_control_t           pm8x41_ldo_control;
  pm8x41_get_pmic_rev_t          pm8x41_get_pmic_rev;
  pm8x41_get_pon_reason_t        pm8x41_get_pon_reason;
  pm8x41_get_pon_poff_reason1_t  pm8x41_get_pon_poff_reason1;
  pm8x41_get_pon_poff_reason2_t  pm8x41_get_pon_poff_reason2;
  pm8x41_get_pwrkey_is_pressed_t pm8x41_get_pwrkey_is_pressed;
  pm8x41_config_output_mpp_t     pm8x41_config_output_mpp;
  pm8x41_enable_mpp_t            pm8x41_enable_mpp;
  pm8x41_enable_mvs_t            pm8x41_enable_mvs;
  pm8x41_get_is_cold_boot_t      pm8x41_get_is_cold_boot;
  pm8x41_diff_clock_ctrl_t       pm8x41_diff_clock_ctrl;
  pm8x41_clear_pmic_watchdog_t   pm8x41_clear_pmic_watchdog;
  pmi8994_config_mpp_slave_id_t  pmi8994_config_mpp_slave_id;
  pm8xxx_is_battery_broken_t     pm8xxx_is_battery_broken;

  pm8x41_adc_channel_read_t      pm8x41_adc_channel_read;
  pm8x41_iusb_max_config_t       pm8x41_iusb_max_config;
  pm8x41_ibat_max_config_t       pm8x41_ibat_max_config;
  pm8x41_chgr_vdd_max_config_t   pm8x41_chgr_vdd_max_config;
  pm8x41_chgr_ctl_enable_t       pm8x41_chgr_ctl_enable;
  pm8x41_get_batt_voltage_t      pm8x41_get_batt_voltage;
  pm8x41_get_voltage_based_soc_t pm8x41_get_voltage_based_soc;
  pm8x41_enable_mpp_as_adc_t     pm8x41_enable_mpp_as_adc;

  pm8x41_reg_write_t             pm8x41_reg_write;
  pm8x41_reg_read_t              pm8x41_reg_read;

  pm8x41_wled_config_t             pm8x41_wled_config;
  pm8x41_wled_iled_sync_control_t  pm8x41_wled_iled_sync_control;
  pm8x41_wled_sink_control_t       pm8x41_wled_sink_control;
  pm8x41_wled_led_mod_enable_t     pm8x41_wled_led_mod_enable;
  pm8x41_wled_enable_t             pm8x41_wled_enable;
  pm8x41_wled_config_slave_id_t    pm8x41_wled_config_slave_id;
  pm8x41_wled_reg_read_t           pm8x41_wled_reg_read;
  pm8x41_wled_reg_write_t          pm8x41_wled_reg_write;

  pm_pwm_enable_t                pm_pwm_enable;
  pm_pwm_config_t                pm_pwm_config;

  pm_vib_turn_on_t               pm_vib_turn_on;
  pm_vib_turn_off_t              pm_vib_turn_off;
};

extern EFI_GUID gQcomPm8x41ProtocolGuid;

#endif
