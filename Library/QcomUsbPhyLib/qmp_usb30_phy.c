/* Copyright (c) 2014-2015, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Library/LKEnvLib.h>

#include <Library/QcomClockLib.h>
#include <Library/QcomUsbPhyLib.h>
// Must come in order
#include <Library/QcomPlatformUsbLib.h>

#include <qmp_phy.h>

#define QMP_PHY_BASE PcdGet64(PcdUsb30QmpPhyBase)

#define HS_PHY_COMMON_CTRL 0xEC
#define USE_CORECLK BIT(14)
#define PLLBTUNE BIT(15)
#define FSEL (0x7 << 4)
#define DIS_RETENTION BIT(18)
#define QMP_PHY_MAX_TIMEOUT 1000
#define PHYSTATUS BIT(6)

static bool hsonly_mode = FALSE;

static struct qmp_reg qmp_settings[] = {
    {0xAC, 0x14},  /* QSERDES_COM_SYSCLK_EN_SEL */
    {0x34, 0x08},  /* QSERDES_COM_BIAS_EN_CLKBUFLR_EN */
    {0x174, 0x30}, /* QSERDES_COM_CLK_SELECT */
    {0x3C, 0x06},  /* QSERDES_COM_SYS_CLK_CTRL */
    {0xb4, 0x00},  /* QSERDES_COM_RESETSM_CNTRL */
    {0xb8, 0x08},  /* QSERDES_COM_RESETSM_CNTRL2 */
    {0x194, 0x06}, /* QSERDES_COM_CMN_CONFIG */
    {0x19c, 0x01}, /* QSERDES_COM_SVS_MODE_CLK_SEL */
    {0x178, 0x01}, /* QSERDES_COM_HSCLK_SEL */
    {0xd0, 0x82},  /* QSERDES_COM_DEC_START_MODE0 */
    {0xdc, 0x55},  /* QSERDES_COM_DIV_FRAC_START1_MODE0 */
    {0xe0, 0x55},  /* QSERDES_COM_DIV_FRAC_START2_MODE0 */
    {0xe4, 0x03},  /* QSERDES_COM_DIV_FRAC_START3_MODE0 */
    {0x78, 0x0b},  /* QSERDES_COM_CP_CTRL_MODE0 */
    {0x84, 0x16},  /* QSERDES_COM_PLL_RCTRL_MODE0 */
    {0x90, 0x28},  /* QSERDES_COM_PLL_CCTRL_MODE0 */
    {0x108, 0x80}, /* QSERDES_COM_INTEGLOOP_GAIN0_MODE0 */
    {0x10c, 0x00}, /* QSERDES_COM_INTEGLOOP_GAIN1_MODE0 */
    {0x124, 0x1c}, /* QSERDES_COM_VCO_TUNE_CTRL */
    {0x12c, 0x3f}, /* QSERDES_COM_VCO_TUNE1_MODE0 */
    {0x130, 0x01}, /* QSERDES_COM_VCO_TUNE2_MODE0 */
    {0x184, 0x0a}, /* QSERDES_COM_CORECLK_DIV */
    {0x4c, 0x15},  /* QSERDES_COM_LOCK_CMP1_MODE0 */
    {0x50, 0x34},  /* QSERDES_COM_LOCK_CMP2_MODE0 */
    {0x54, 0x00},  /* QSERDES_COM_LOCK_CMP3_MODE0 */
    {0xc8, 0x00},  /* QSERDES_COM_LOCK_CMP_EN */
    {0x18c, 0x00}, /* QSERDES_COM_CORE_CLK_EN */
    {0xc4, 0x15},  /*QSERDES_COM_RESCODE_DIV_NUM */
    {0xcc, 0x00},  /* QSERDES_COM_LOCK_CMP_CFG */
    {0x128, 0x00}, /* QSERDES_COM_VCO_TUNE_MAP */
    {0xc, 0x0a},   /* QSERDES_COM_BG_TIMER */
    {0x10, 0x01},  /* QSERDES_COM_SSC_EN_CENTER */
    {0x1c, 0x31},  /* QSERDES_COM_SSC_PER1 */
    {0x20, 0x01},  /* QSERDES_COM_SSC_PER2 */
    {0x14, 0x00},  /* QSERDES_COM_SSC_ADJ_PER1 */
    {0x18, 0x00},  /* QSERDES_COM_SSC_ADJ_PER2 */
    {0x24, 0xde},  /* QSERDES_COM_SSC_STEP_SIZE1 */
    {0x28, 0x07},  /* QSERDES_COM_SSC_STEP_SIZE2 */
    {0x440, 0x0b}, /* QSERDES_RX_UCDR_FASTLOCK_FO_GAIN */
    {0x4d8, 0x03}, /* QSERDES_RX_RX_EQU_ADAPTOR_CNTRL2 */
    {0x4dc, 0x6c}, /* QSERDES_RX_RX_EQU_ADAPTOR_CNTRL3 */
    {0x70, 0xf},   /* QSERDES_COM_BG_TRIM */
    {0x48, 0xf},   /* QSERDES_COM_PLL_IVCO */
    {0x4e0, 0xb8}, /* QSERDES_RX_RX_EQU_ADAPTOR_CNTRL4 */
    {0x508, 0x77}, /* QSERDES_RX_RX_EQ_OFFSET_ADAPTOR_CNTRL1 */
    {0x50c, 0x80}, /* QSERDES_RX_RX_OFFSET_ADAPTOR_CNTRL2 */
    {0x514, 0x04}, /* QSERDES_RX_SIGDET_CNTRL */
    {0x518, 0x1b}, /* QSERDES_RX_SIGDET_LVL */
    {0x51c, 0x16}, /* QSERDES_RX_SIGDET_DEGLITCH_CNTRL */
    {0x268, 0x45}, /* QSERDES_TX_HIGHZ_TRANSCEIVEREN_BIAS_DRVR_EN */
    {0x2ac, 0x12}, /* QSERDES_TX_RCV_DETECT_LVL_2 */
    {0x6c4, 0x03}, /* USB3_PHY_FLL_CNTRL2 */
    {0x6c0, 0x02}, /* USB3_PHY_FLL_CNTRL1 */
    {0x6c8, 0x09}, /* USB3_PHY_FLL_CNT_VAL_L */
    {0x6cc, 0x42}, /* USB3_PHY_FLL_CNT_VAL_H_TOL */
    {0x6d0, 0x85}, /* USB3_PHY_FLL_MAN_CODE */
    {0x294, 0x02}, /* QSERDES_TX_LANE_MODE */
    {0x680, 0xd1}, /* USB3_PHY_LOCK_DETECT_CONFIG1 */
    {0x684, 0x1f}, /* USB3_PHY_LOCK_DETECT_CONFIG2 */
    {0x688, 0x47}, /* USB3_PHY_LOCK_DETECT_CONFIG3 */
    {0x664, 0x08}, /* USB3_PHY_POWER_STATE_CONFIG2 */
    {0x600, 0x00}, /* USB3_PHY_SW_RESET */
    {0x608, 0x03}, /* USB3_PHY_START_CONTROL */
};

/* USB3.0 QMP phy reset */
void qmp_phy_qmp_reset(target_usb_iface_t *iface)
{
  int      ret = 0;
  uint32_t val;
  bool     phy_com_reset = false;

  struct clk *usb2b_clk    = NULL;
  struct clk *usb_pipe_clk = NULL;
  struct clk *phy_com_clk  = NULL;
  struct clk *phy_clk      = NULL;

  /* Look if phy com clock is present */
  phy_com_clk = gClock->clk_get("usb30_phy_com_reset");
  if (phy_com_clk)
    phy_com_reset = true;

  usb2b_clk = gClock->clk_get("usb2b_phy_sleep_clk");
  ASSERT(usb2b_clk);

  phy_clk = gClock->clk_get("usb30_phy_reset");
  ASSERT(phy_clk);

  usb_pipe_clk = gClock->clk_get("usb30_pipe_clk");
  ASSERT(usb_pipe_clk);

  /* ASSERT */
  ret = gClock->clk_reset(usb2b_clk, CLK_RESET_ASSERT);
  if (ret) {
    dprintf(CRITICAL, "Failed to assert usb2b_phy_clk\n");
    return;
  }

  if (phy_com_reset) {
    ret = gClock->clk_reset(phy_com_clk, CLK_RESET_ASSERT);
    if (ret) {
      dprintf(CRITICAL, "Failed to assert phy_com_clk\n");
      goto deassert_usb2b_clk;
    }
  }

  ret = gClock->clk_reset(phy_clk, CLK_RESET_ASSERT);
  if (ret) {
    dprintf(CRITICAL, "Failed to assert phy_clk\n");
    goto deassert_phy_com_clk;
  }

  ret = gClock->clk_reset(usb_pipe_clk, CLK_RESET_ASSERT);
  if (ret) {
    dprintf(CRITICAL, "Failed to assert usb_pipe_clk\n");
    goto deassert_phy_clk;
  }

  udelay(100);

  /* DEASSERT */
  ret = gClock->clk_reset(usb_pipe_clk, CLK_RESET_DEASSERT);
  if (ret)
    dprintf(CRITICAL, "Failed to deassert usb_pipe_clk\n");

deassert_phy_clk:
  ret = gClock->clk_reset(phy_clk, CLK_RESET_DEASSERT);
  if (ret)
    dprintf(CRITICAL, "Failed to deassert phy_clk\n");

deassert_phy_com_clk:
  if (phy_com_reset) {
    ret = gClock->clk_reset(phy_com_clk, CLK_RESET_DEASSERT);
    if (ret)
      dprintf(CRITICAL, "Failed to deassert phy_com_clk\n");
  }

deassert_usb2b_clk:
  ret = gClock->clk_reset(usb2b_clk, CLK_RESET_DEASSERT);
  if (ret)
    dprintf(CRITICAL, "Failed to deassert usb2b_phy_clk\n");

  /* Override the phy common control values */
  val = readl(PcdGet64(PcdUsb30ScratchBase) + HS_PHY_COMMON_CTRL);
  val |= USE_CORECLK | PLLBTUNE;
  val &= ~FSEL;
  val &= ~DIS_RETENTION;
  writel(val, PcdGet64(PcdUsb30ScratchBase) + HS_PHY_COMMON_CTRL);
}

/* USB 3.0 phy init: HPG for QMP phy*/
void usb30_qmp_phy_init(target_usb_iface_t *iface)
{
  int      timeout    = QMP_PHY_MAX_TIMEOUT;
  uint32_t rev_id     = 0;
  uint32_t phy_status = 0;
  uint32_t qmp_reg_size;
  uint32_t i;

  if (iface->get_qmp_rev)
    rev_id = iface->get_qmp_rev(iface);
  else
    rev_id = 0x10000000;

  /* Sequence as per HPG */
  writel(0x01, QMP_PHY_BASE + PCIE_USB3_PHY_POWER_DOWN_CONTROL);

  if (rev_id >= 0x20000000) {
    qmp_reg_size = sizeof(qmp_settings) / sizeof(struct qmp_reg);
    for (i = 0; i < qmp_reg_size; i++)
      writel(qmp_settings[i].val, QMP_PHY_BASE + qmp_settings[i].off);
  }
  else {
    writel(0x08, QMP_PHY_BASE + QSERDES_COM_SYSCLK_EN_SEL_TXBAND);

    if (iface->pll_override)
      writel(0xE1, QMP_PHY_BASE + QSERDES_COM_PLL_VCOTAIL_EN);

    writel(0x82, QMP_PHY_BASE + QSERDES_COM_DEC_START1);
    writel(0x03, QMP_PHY_BASE + QSERDES_COM_DEC_START2);
    writel(0xD5, QMP_PHY_BASE + QSERDES_COM_DIV_FRAC_START1);
    writel(0xAA, QMP_PHY_BASE + QSERDES_COM_DIV_FRAC_START2);
    writel(0x4D, QMP_PHY_BASE + QSERDES_COM_DIV_FRAC_START3);
    writel(0x01, QMP_PHY_BASE + QSERDES_COM_PLLLOCK_CMP_EN);
    writel(0x2B, QMP_PHY_BASE + QSERDES_COM_PLLLOCK_CMP1);
    writel(0x68, QMP_PHY_BASE + QSERDES_COM_PLLLOCK_CMP2);
    writel(0x7C, QMP_PHY_BASE + QSERDES_COM_PLL_CRCTRL);
    writel(0x02, QMP_PHY_BASE + QSERDES_COM_PLL_CP_SETI);
    writel(0x1F, QMP_PHY_BASE + QSERDES_COM_PLL_IP_SETP);
    writel(0x0F, QMP_PHY_BASE + QSERDES_COM_PLL_CP_SETP);
    writel(0x01, QMP_PHY_BASE + QSERDES_COM_PLL_IP_SETI);
    writel(0x0F, QMP_PHY_BASE + QSERDES_COM_IE_TRIM);
    writel(0x0F, QMP_PHY_BASE + QSERDES_COM_IP_TRIM);
    writel(0x46, QMP_PHY_BASE + QSERDES_COM_PLL_CNTRL);

    /* CDR Settings */
    writel(0xDA, QMP_PHY_BASE + QSERDES_RX_CDR_CONTROL1);
    writel(0x42, QMP_PHY_BASE + QSERDES_RX_CDR_CONTROL2);

    /* Calibration Settings */
    writel(0x90, QMP_PHY_BASE + QSERDES_COM_RESETSM_CNTRL);
    if (iface->pll_override)
      writel(0x07, QMP_PHY_BASE + QSERDES_COM_RESETSM_CNTRL2);
    else
      writel(0x05, QMP_PHY_BASE + QSERDES_COM_RESETSM_CNTRL2);

    writel(0x20, QMP_PHY_BASE + QSERDES_COM_RES_CODE_START_SEG1);
    writel(0x77, QMP_PHY_BASE + QSERDES_COM_RES_CODE_CAL_CSR);
    writel(0x15, QMP_PHY_BASE + QSERDES_COM_RES_TRIM_CONTROL);
    writel(0x03, QMP_PHY_BASE + QSERDES_TX_RCV_DETECT_LVL);
    writel(0x02, QMP_PHY_BASE + QSERDES_RX_RX_EQU_ADAPTOR_CNTRL2);
    writel(0x6C, QMP_PHY_BASE + QSERDES_RX_RX_EQU_ADAPTOR_CNTRL3);
    writel(0xC7, QMP_PHY_BASE + QSERDES_RX_RX_EQU_ADAPTOR_CNTRL4);
    writel(0x40, QMP_PHY_BASE + QSERDES_RX_SIGDET_ENABLES);
    writel(0x73, QMP_PHY_BASE + QSERDES_RX_SIGDET_CNTRL);
    writel(0x06, QMP_PHY_BASE + QSERDES_RX_SIGDET_DEGLITCH_CNTRL);
    writel(0x48, QMP_PHY_BASE + PCIE_USB3_PHY_RX_IDLE_DTCT_CNTRL);
    writel(0x01, QMP_PHY_BASE + QSERDES_COM_SSC_EN_CENTER);
    writel(0x02, QMP_PHY_BASE + QSERDES_COM_SSC_ADJ_PER1);
    writel(0x31, QMP_PHY_BASE + QSERDES_COM_SSC_PER1);
    writel(0x01, QMP_PHY_BASE + QSERDES_COM_SSC_PER2);
    writel(0x19, QMP_PHY_BASE + QSERDES_COM_SSC_STEP_SIZE1);
    writel(0x19, QMP_PHY_BASE + QSERDES_COM_SSC_STEP_SIZE2);
    writel(0x08, QMP_PHY_BASE + PCIE_USB3_PHY_POWER_STATE_CONFIG2);

    writel(0x00, QMP_PHY_BASE + PCIE_USB3_PHY_SW_RESET);
    writel(0x03, QMP_PHY_BASE + PCIE_USB3_PHY_START);
  }

  if (rev_id >= 0x20000000)
    phy_status = 0x77c;
  else
    phy_status = 0x728;

  while ((readl(QMP_PHY_BASE + phy_status) & PHYSTATUS)) {
    udelay(1);
    timeout--;
    if (!timeout) {
      dprintf(
          CRITICAL,
          "QMP phy initialization failed, fallback to HighSpeed only mode\n");
      hsonly_mode = true;
      return;
    }
  }

  if (iface->clock_bumpup_pipe3_clk)
    iface->clock_bumpup_pipe3_clk(iface);
}

bool use_hsonly_mode(void) { return hsonly_mode; }
