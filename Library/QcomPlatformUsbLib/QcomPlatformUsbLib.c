#include <Base.h>
#include <Chipset/regulator.h>
#include <Library/LKEnvLib.h>
#include <Library/QcomBoardLib.h>
#include <Library/QcomClockLib.h>
#include <Library/QcomGpioTlmmLib.h>
#include <Library/QcomPlatformUsbLib.h>
#include <Library/QcomRpmLib.h>
#include <Library/QcomUsbPhyLib.h>
#include <Platform/iomap.h>

/* Enable LN BB CLK */
STATIC UINT32 ln_bb_clk[][8] = {
    {
        RPM_CLK_BUFFER_A_REQ,
        LNBB_CLK_ID,
        KEY_SOFTWARE_ENABLE,
        4,
        GENERIC_DISABLE,
        RPM_KEY_PIN_CTRL_CLK_BUFFER_ENABLE_KEY,
        4,
        RPM_CLK_BUFFER_PIN_CONTROL_ENABLE_NONE,
    },
    {
        RPM_CLK_BUFFER_A_REQ,
        LNBB_CLK_ID,
        KEY_SOFTWARE_ENABLE,
        4,
        GENERIC_ENABLE,
        RPM_KEY_PIN_CTRL_CLK_BUFFER_ENABLE_KEY,
        4,
        RPM_CLK_BUFFER_PIN_CONTROL_ENABLE_NONE,
    },
};

/* api to control lnbb clock */
STATIC VOID pm8x41_lnbb_clock_ctrl(UINT8 enable)
{
  if (enable) {
    gRpm->rpm_clk_enable(&ln_bb_clk[GENERIC_ENABLE][0], 24);
  }
  else {
    gRpm->rpm_clk_enable(&ln_bb_clk[GENERIC_DISABLE][0], 24);
  }
}

STATIC VOID clock_usb30_gdsc_enable(VOID)
{
  UINT32 reg = readl(GCC_USB30_GDSCR);

  reg &= ~(0x1);

  writel(reg, GCC_USB30_GDSCR);
}

/* enables usb30 clocks */
STATIC VOID clock_usb30_init(target_usb_iface_t *iface)
{
  int ret;

  ret = gClock->clk_get_set_enable("usb30_iface_clk", 0, 1);
  if (ret) {
    dprintf(CRITICAL, "failed to set usb30_iface_clk. ret = %d\n", ret);
    ASSERT(0);
  }

  clock_usb30_gdsc_enable();

  ret = gClock->clk_get_set_enable("usb30_master_clk", 125000000, 1);
  if (ret) {
    dprintf(CRITICAL, "failed to set usb30_master_clk. ret = %d\n", ret);
    ASSERT(0);
  }

  ret = gClock->clk_get_set_enable("usb30_phy_aux_clk", 1200000, 1);
  if (ret) {
    dprintf(CRITICAL, "failed to set usb30_phy_aux_clk. ret = %d\n", ret);
    ASSERT(0);
  }

  ret = gClock->clk_get_set_enable("usb30_mock_utmi_clk", 60000000, 1);
  if (ret) {
    dprintf(CRITICAL, "failed to set usb30_mock_utmi_clk ret = %d\n", ret);
    ASSERT(0);
  }

  ret = gClock->clk_get_set_enable("usb30_sleep_clk", 0, 1);
  if (ret) {
    dprintf(CRITICAL, "failed to set usb30_sleep_clk ret = %d\n", ret);
    ASSERT(0);
  }

  ret = gClock->clk_get_set_enable("usb_phy_cfg_ahb2phy_clk", 0, 1);
  if (ret) {
    dprintf(CRITICAL, "failed to enable usb_phy_cfg_ahb2phy_clk = %d\n", ret);
    ASSERT(0);
  }

  pm8x41_lnbb_clock_ctrl(1);
}

/* mux hs phy to route to dwc controller */
STATIC VOID phy_mux_configure_with_tcsr(VOID)
{
  /* As per the hardware team, set the mux for snps controller */
  RMWREG32(TCSR_PHSS_USB2_PHY_SEL, 0x0, 0x1, 0x1);
}

/* configure hs phy mux if using dwc controller */
STATIC VOID target_usb_phy_mux_configure(target_usb_iface_t *iface)
{
  phy_mux_configure_with_tcsr();
}

STATIC VOID clock_bumpup_pipe3_clk(target_usb_iface_t *iface)
{
  INTN ret = 0;

  ret = gClock->clk_get_set_enable("usb30_pipe_clk", 0, 1);
  if (ret) {
    DEBUG((DEBUG_ERROR, "failed to set usb30_pipe_clk. ret = %d\n", ret));
    ASSERT(0);
  }
}

EFI_STATUS LibQcomPlatformUsbGetInterface(target_usb_iface_t *iface)
{
  iface->controller             = L"dwc";
  iface->mux_config             = target_usb_phy_mux_configure;
  iface->phy_init               = usb30_qmp_phy_init;
  iface->phy_reset              = qmp_phy_qmp_reset;
  iface->clock_init             = clock_usb30_init;
  iface->clock_bumpup_pipe3_clk = clock_bumpup_pipe3_clk;
  iface->vbus_override          = TRUE;
  iface->pll_override           = TRUE;

  return EFI_SUCCESS;
}
