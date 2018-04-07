#include <Base.h>

#include <Library/LKEnvLib.h>
#include <Library/QcomSpmiLib.h>
#include <Library/QcomPm8x41Lib.h>

#include "pm8x41_p.h"

QCOM_PM8X41_PROTOCOL *gPm8x41 = NULL;

STATIC QCOM_PM8X41_PROTOCOL mInternalPm8x41 = {
  pm8x41_lpg_write,
  pm8x41_lpg_write_sid,
  pm8x41_gpio_get,
  pm8x41_gpio_get_sid,
  pm8x41_gpio_set,
  pm8x41_gpio_set_sid,
  pm8x41_gpio_config,
  pm8x41_gpio_config_sid,
  pm8x41_set_boot_done,
  pm8x41_v2_resin_status,
  pm8x41_resin_status,
  pm8x41_reset_configure,
  pm8994_reset_configure,
  pm8x41_v2_reset_configure,
  pm8x41_ldo_set_voltage,
  pm8x41_ldo_control,
  pm8x41_get_pmic_rev,
  pm8x41_get_pon_reason,
  pm8x41_get_pon_poff_reason1,
  pm8x41_get_pon_poff_reason2,
  pm8x41_get_pwrkey_is_pressed,
  pm8x41_config_output_mpp,
  pm8x41_enable_mpp,
  pm8x41_enable_mvs,
  pm8x41_get_is_cold_boot,
  pm8x41_diff_clock_ctrl,
  pm8x41_clear_pmic_watchdog,
  pmi8994_config_mpp_slave_id,
  pm8xxx_is_battery_broken,

  pm8x41_adc_channel_read,
  pm8x41_iusb_max_config,
  pm8x41_ibat_max_config,
  pm8x41_chgr_vdd_max_config,
  pm8x41_chgr_ctl_enable,
  pm8x41_get_batt_voltage,
  pm8x41_get_voltage_based_soc,
  pm8x41_enable_mpp_as_adc,

  pm8x41_reg_write,
  pm8x41_reg_read,

  pm8x41_wled_config,
  pm8x41_wled_iled_sync_control,
  pm8x41_wled_sink_control,
  pm8x41_wled_led_mod_enable,
  pm8x41_wled_enable,
  pm8x41_wled_config_slave_id,
  pm8x41_wled_reg_read,
  pm8x41_wled_reg_write,

  pm_pwm_enable,
  pm_pwm_config,

  pm_vib_turn_on,
  pm_vib_turn_off,
};

RETURN_STATUS
EFIAPI
Pm8x41ImplLibInitialize (
  VOID
  )
{
  gPm8x41 = &mInternalPm8x41;

  return RETURN_SUCCESS;
}
