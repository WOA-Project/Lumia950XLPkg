#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomGpioTlmmLib.h>
#include <Library/QcomTargetMmcSdhciLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Chipset/mmc_sdhci.h>

#include <Platform/iomap.h>
#include <Platform/irqs.h>

static uint32_t mmc_pwrctl_base[] = {MSM_SDC1_BASE, MSM_SDC2_BASE};

static uint32_t mmc_sdhci_base[] = {MSM_SDC1_SDHCI_BASE, MSM_SDC2_SDHCI_BASE};

static uint32_t mmc_sdc_pwrctl_irq[] = {SDCC1_PWRCTL_IRQ, SDCC2_PWRCTL_IRQ};

VOID LibQcomTargetMmcSdhciInit(INIT_SLOT_CB InitSlot)
{
  uint32_t               reg = 0;
  uint8_t                clk;
  uint8_t                cmd;
  uint8_t                dat;
  struct mmc_config_data config = {0};

  // Init eMMC slot
  config.bus_width    = DATA_BUS_WIDTH_8BIT;
  config.max_clk_rate = MMC_CLK_192MHZ;

  /* Try slot 1 */
  config.slot          = 1;
  config.sdhc_base     = mmc_sdhci_base[config.slot - 1];
  config.pwrctl_base   = mmc_pwrctl_base[config.slot - 1];
  config.pwr_irq       = mmc_sdc_pwrctl_irq[config.slot - 1];
  config.hs200_support = 0;

  clk = TLMM_CUR_VAL_10MA;
  cmd = TLMM_CUR_VAL_8MA;
  dat = TLMM_CUR_VAL_8MA;
  reg = SDC1_HDRV_PULL_CTL;

  /* Drive strength configs for sdc pins */
  struct tlmm_cfgs sdc1_hdrv_cfg[] = {
      {SDC1_CLK_HDRV_CTL_OFF, clk, TLMM_HDRV_MASK, reg},
      {SDC1_CMD_HDRV_CTL_OFF, cmd, TLMM_HDRV_MASK, reg},
      {SDC1_DATA_HDRV_CTL_OFF, dat, TLMM_HDRV_MASK, reg},
  };

  /* Pull configs for sdc pins */
  struct tlmm_cfgs sdc1_pull_cfg[] = {
      {SDC1_CLK_PULL_CTL_OFF, TLMM_NO_PULL, TLMM_PULL_MASK, reg},
      {SDC1_CMD_PULL_CTL_OFF, TLMM_PULL_UP, TLMM_PULL_MASK, reg},
      {SDC1_DATA_PULL_CTL_OFF, TLMM_PULL_UP, TLMM_PULL_MASK, reg},
  };

  struct tlmm_cfgs sdc1_rclk_cfg[] = {
      {SDC1_RCLK_PULL_CTL_OFF, TLMM_PULL_DOWN, TLMM_PULL_MASK, reg},
  };

  /* Set the drive strength & pull control values */
  gGpioTlmm->tlmm_set_hdrive_ctrl(sdc1_hdrv_cfg, ARRAY_SIZE(sdc1_hdrv_cfg));
  gGpioTlmm->tlmm_set_pull_ctrl(sdc1_pull_cfg, ARRAY_SIZE(sdc1_pull_cfg));
  gGpioTlmm->tlmm_set_pull_ctrl(sdc1_rclk_cfg, ARRAY_SIZE(sdc1_rclk_cfg));

  if (InitSlot(&config) == NULL) {
    DEBUG((DEBUG_ERROR, "Can't initialize mmc slot %u\n", config.slot));
  }

  // Init SD card slot
  config.bus_width     = DATA_BUS_WIDTH_4BIT;
  config.max_clk_rate  = MMC_CLK_200MHZ;
  config.slot          = 2;
  config.sdhc_base     = mmc_sdhci_base[config.slot - 1];
  config.pwrctl_base   = mmc_pwrctl_base[config.slot - 1];
  config.pwr_irq       = mmc_sdc_pwrctl_irq[config.slot - 1];
  config.hs200_support = 0;

  clk = TLMM_CUR_VAL_10MA;
  cmd = TLMM_CUR_VAL_8MA;
  dat = TLMM_CUR_VAL_8MA;
  reg = SDC2_HDRV_PULL_CTL;

  struct tlmm_cfgs sdc2_hdrv_cfg[] = {
      {SDC1_CLK_HDRV_CTL_OFF, clk, TLMM_HDRV_MASK, reg},
      {SDC1_CMD_HDRV_CTL_OFF, cmd, TLMM_HDRV_MASK, reg},
      {SDC1_DATA_HDRV_CTL_OFF, dat, TLMM_HDRV_MASK, reg},
  };

  struct tlmm_cfgs sdc2_pull_cfg[] = {
      {SDC1_CLK_PULL_CTL_OFF, TLMM_NO_PULL, TLMM_PULL_MASK, reg},
      {SDC1_CMD_PULL_CTL_OFF, TLMM_PULL_UP, TLMM_PULL_MASK, reg},
      {SDC1_DATA_PULL_CTL_OFF, TLMM_PULL_UP, TLMM_PULL_MASK, reg},
  };

  struct tlmm_cfgs sdc2_rclk_cfg[] = {
      {SDC1_RCLK_PULL_CTL_OFF, TLMM_PULL_DOWN, TLMM_PULL_MASK, reg},
  };

  /* Set the drive strength & pull control values */
  gGpioTlmm->tlmm_set_hdrive_ctrl(sdc2_hdrv_cfg, ARRAY_SIZE(sdc2_hdrv_cfg));
  gGpioTlmm->tlmm_set_pull_ctrl(sdc2_pull_cfg, ARRAY_SIZE(sdc2_pull_cfg));
  gGpioTlmm->tlmm_set_pull_ctrl(sdc2_rclk_cfg, ARRAY_SIZE(sdc2_rclk_cfg));

  if (InitSlot(&config) == NULL) {
    DEBUG((DEBUG_ERROR, "Can't initialize mmc slot %u\n", config.slot));
  }
}
