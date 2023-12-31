#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/ArmLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <GplLibrary/clock-gcc-msm8994.h>
#include <GplLibrary/clock-rpm-msm8994.h>
#include <GplLibrary/gdsc-msm8994.h>

#include <Platform/iomap.h>

#include <Protocol/QcomBoard.h>
#include <Protocol/QcomGpioTlmm.h>
#include <Protocol/QcomPm8x41.h>
#include <Protocol/QcomRpm.h>
#include <Protocol/QcomClock.h>

#include "Usb3Phy.h"

static QCOM_GPIO_TLMM_PROTOCOL *mTlmmProtocol;
static QCOM_RPM_PROTOCOL *mRpmProtocol;
static QCOM_PM8X41_PROTOCOL *mPmicProtocol;
static QCOM_BOARD_PROTOCOL *mBoardProtocol;
static QCOM_CLOCK_PROTOCOL *mClockProtocol;

static uint32_t mBoardId = 0;
static bool hsonly_mode;

#define HS_PHY_COMMON_CTRL             0xEC
#define USE_CORECLK                    BIT(14)
#define PLLBTUNE                       BIT(15)
#define FSEL                           (0x7 << 4)
#define DIS_RETENTION                  BIT(18)
#define QMP_PHY_MAX_TIMEOUT            1000
#define PHYSTATUS                      BIT(6)

/* Override values for QMP 2.0 V1 devices */
struct qmp_reg qmp_override_pll_rev2[] =
{
	{0x124, 0x1C}, /* USB3PHY_QSERDES_COM_VCO_TUNE_CTRL */
	{0x12C, 0x3F}, /* USB3PHY_QSERDES_COM_VCO_TUNE1_MODE0 */
	{0x130, 0x01}, /* USB3PHY_QSERDES_COM_VCO_TUNE2_MODE0 */
	{0x6c4, 0x13}, /* USB3_PHY_FLL_CNTRL2 */
};

/* QMP settings for 2.0 QMP V2 HW */
struct qmp_reg qmp_settings_rev2[] =
{
	{0xAC, 0x14}, /* QSERDES_COM_SYSCLK_EN_SEL */
	{0x34, 0x08}, /* QSERDES_COM_BIAS_EN_CLKBUFLR_EN */
	{0x174, 0x30}, /* QSERDES_COM_CLK_SELECT */
	{0x194, 0x06}, /* QSERDES_COM_CMN_CONFIG */
	{0x19c, 0x01}, /* QSERDES_COM_SVS_MODE_CLK_SEL */
	{0x178, 0x01}, /* QSERDES_COM_HSCLK_SEL */
	{0x70, 0x0F}, /* USB3PHY_QSERDES_COM_BG_TRIM */
	{0x48, 0x0F}, /* USB3PHY_QSERDES_COM_PLL_IVCO */
	{0x3C, 0x04}, /* QSERDES_COM_SYS_CLK_CTRL */

	/* Res_code Settings */
	{0xC4, 0x15}, /* USB3PHY_QSERDES_COM_RESCODE_DIV_NUM */
	{0x1B8, 0x1F}, /* QSERDES_COM_CMN_MISC2 */

	/* PLL & Loop filter settings */
	{0xd0, 0x82}, /* QSERDES_COM_DEC_START_MODE0 */
	{0xdc, 0x55}, /* QSERDES_COM_DIV_FRAC_START1_MODE0 */
	{0xe0, 0x55}, /* QSERDES_COM_DIV_FRAC_START2_MODE0 */
	{0xe4, 0x03}, /* QSERDES_COM_DIV_FRAC_START3_MODE0 */
	{0x78, 0x0b}, /* QSERDES_COM_CP_CTRL_MODE0 */
	{0x84, 0x16}, /* QSERDES_COM_PLL_RCTRL_MODE0 */
	{0x90, 0x28}, /* QSERDES_COM_PLL_CCTRL_MODE0 */
	{0x108, 0x80}, /* QSERDES_COM_INTEGLOOP_GAIN0_MODE0 */
	{0x124, 0x00}, /* QSERDES_COM_VCO_TUNE_CTRL */
	{0x4c, 0x15}, /* QSERDES_COM_LOCK_CMP1_MODE0 */
	{0x50, 0x34}, /* QSERDES_COM_LOCK_CMP2_MODE0 */
	{0x54, 0x00}, /* QSERDES_COM_LOCK_CMP3_MODE0 */
	{0x18c, 0x00}, /* QSERDES_COM_CORE_CLK_EN */
	{0xcc, 0x00}, /* QSERDES_COM_LOCK_CMP_CFG */
	{0x0C, 0x0A}, /* QSERDES_COM_BG_TIMER */
	{0x128, 0x00}, /* QSERDES_COM_VCO_TUNE_MAP */
	{0xc, 0x0a}, /* QSERDES_COM_BG_TIMER */

	/* SSC settings */
	{0x10, 0x01}, /* QSERDES_COM_SSC_EN_CENTER */
	{0x1c, 0x31}, /* QSERDES_COM_SSC_PER1 */
	{0x20, 0x01}, /* QSERDES_COM_SSC_PER2 */
	{0x14, 0x00}, /* QSERDES_COM_SSC_ADJ_PER1 */
	{0x18, 0x00}, /* QSERDES_COM_SSC_ADJ_PER2 */
	{0x24, 0xde}, /* QSERDES_COM_SSC_STEP_SIZE1 */
	{0x28, 0x07}, /* QSERDES_COM_SSC_STEP_SIZE2 */

	/* Rx Settings */
	{0x440, 0x0b}, /* QSERDES_RX_UCDR_FASTLOCK_FO_GAIN */
	{0x41C, 0x04}, /* QSERDES_RX_UCDR_SO_GAIN */
	{0x4d8, 0x02}, /* QSERDES_RX_RX_EQU_ADAPTOR_CNTRL2 */
	{0x4dc, 0x4c}, /* QSERDES_RX_RX_EQU_ADAPTOR_CNTRL3 */
	{0x4e0, 0xbb}, /* QSERDES_RX_RX_EQU_ADAPTOR_CNTRL4 */
	{0x508, 0x77}, /* QSERDES_RX_RX_EQ_OFFSET_ADAPTOR_CNTRL1 */
	{0x50c, 0x80}, /* QSERDES_RX_RX_OFFSET_ADAPTOR_CNTRL2 */
	{0x514, 0x03}, /* QSERDES_RX_SIGDET_CNTRL */
	{0x518, 0x1b}, /* QSERDES_RX_SIGDET_LVL */
	{0x51c, 0x16}, /* QSERDES_RX_SIGDET_DEGLITCH_CNTRL */

	/* Tx settings */
	{0x268, 0x45}, /* QSERDES_TX_HIGHZ_TRANSCEIVEREN_BIAS_DRVR_EN */
	{0x2ac, 0x12}, /* QSERDES_TX_RCV_DETECT_LVL_2 */
	{0x294, 0x06}, /* QSERDES_TX_LANE_MODE */

	/* FLL settings */
	{0x6c4, 0x03}, /* USB3_PHY_FLL_CNTRL2 */
	{0x6c0, 0x02}, /* USB3_PHY_FLL_CNTRL1 */
	{0x6c8, 0x09}, /* USB3_PHY_FLL_CNT_VAL_L */
	{0x6cc, 0x42}, /* USB3_PHY_FLL_CNT_VAL_H_TOL */
	{0x6d0, 0x85}, /* USB3_PHY_FLL_MAN_CODE */

	/* Lock Det Settings */
	{0x680, 0xd1}, /* USB3_PHY_LOCK_DETECT_CONFIG1 */
	{0x684, 0x1f}, /* USB3_PHY_LOCK_DETECT_CONFIG2 */
	{0x688, 0x47}, /* USB3_PHY_LOCK_DETECT_CONFIG3 */
	{0x664, 0x08}, /* USB3_PHY_POWER_STATE_CONFIG2 */
	{0x608, 0x03}, /* USB3_PHY_START_CONTROL */
	{0x600, 0x00}, /* USB3_PHY_SW_RESET */
};

uint32_t target_override_pll()
{
	return 1;
}

uint32_t platform_get_qmp_rev()
{
	return 0x10000000;
}

static void qmp_phy_qmp_reset(void)
{
    int ret = 0;
	uint32_t val;
	bool phy_com_reset = false;

    struct clk *usb2b_clk = NULL;
	struct clk *usb_pipe_clk = NULL;
	struct clk *phy_com_clk = NULL;
	struct clk *phy_clk = NULL;

    phy_com_clk = mClockProtocol->clk_get("usb30_phy_com_reset");
    if (phy_com_clk) phy_com_reset = true;

    usb2b_clk = mClockProtocol->clk_get("usb2b_phy_sleep_clk");
	ASSERT(usb2b_clk != NULL);

	phy_clk  = mClockProtocol->clk_get("usb30_phy_reset");
	ASSERT(phy_clk != NULL);

	usb_pipe_clk = mClockProtocol->clk_get("usb30_pipe_clk");
	ASSERT(usb_pipe_clk != NULL);

    /* ASSERT */
	ret = mClockProtocol->clk_reset(usb2b_clk, CLK_RESET_ASSERT);
	if (ret)
	{
		dprintf(CRITICAL, "Failed to assert usb2b_phy_clk\n");
		return;
	}

	if (phy_com_reset)
	{
		ret = mClockProtocol->clk_reset(phy_com_clk, CLK_RESET_ASSERT);
		if (ret)
		{
			dprintf(CRITICAL, "Failed to assert phy_com_clk\n");
			goto deassert_usb2b_clk;
		}
	}

	ret = mClockProtocol->clk_reset(phy_clk, CLK_RESET_ASSERT);
	if (ret)
	{
		dprintf(CRITICAL, "Failed to assert phy_clk\n");
		goto deassert_phy_com_clk;
	}

	ret = mClockProtocol->clk_reset(usb_pipe_clk, CLK_RESET_ASSERT);
	if (ret)
	{
		dprintf(CRITICAL, "Failed to assert usb_pipe_clk\n");
		goto deassert_phy_clk;
	}

	udelay(100);

	/* DEASSERT */
	ret = mClockProtocol->clk_reset(usb_pipe_clk, CLK_RESET_DEASSERT);
	if (ret)
		dprintf(CRITICAL, "Failed to deassert usb_pipe_clk\n");

deassert_phy_clk:
	ret = mClockProtocol->clk_reset(phy_clk, CLK_RESET_DEASSERT);
	if (ret)
		dprintf(CRITICAL, "Failed to deassert phy_clk\n");

deassert_phy_com_clk:
	if (phy_com_reset)
	{
		ret = mClockProtocol->clk_reset(phy_com_clk, CLK_RESET_DEASSERT);
		if (ret)
			dprintf(CRITICAL, "Failed to deassert phy_com_clk\n");
	}

deassert_usb2b_clk:
	ret = mClockProtocol->clk_reset(usb2b_clk, CLK_RESET_DEASSERT);
	if (ret)
		dprintf(CRITICAL, "Failed to deassert usb2b_phy_clk\n");

	/* Override the phy common control values */
	val = readl(MSM_USB30_QSCRATCH_BASE + HS_PHY_COMMON_CTRL);
	val |= USE_CORECLK | PLLBTUNE;
	val &= ~FSEL;
	val &= ~DIS_RETENTION;
	writel(val, MSM_USB30_QSCRATCH_BASE + HS_PHY_COMMON_CTRL);
}

/* USB3.0 QMP phy reset */
void usb30_qmp_phy_reset(void)
{
#if USB_RESET_FROM_CLK
	clock_reset_usb_phy();
#else
	qmp_phy_qmp_reset();
#endif
}

void clock_bumpup_pipe3_clk()
{
	int ret = 0;

	ret = mClockProtocol->clk_get_set_enable("usb30_pipe_clk", 0, 1);
	if(ret)
	{
		dprintf(CRITICAL, "failed to set usb30_pipe_clk. ret = %d\n", ret);
		ASSERT(0);
	}

	return;
}

/* USB 3.0 phy init: HPG for QMP phy*/
void usb30_qmp_phy_init()
{
	int timeout = QMP_PHY_MAX_TIMEOUT;
	uint32_t rev_id = 0;
	uint32_t phy_status = 0;
	uint32_t qmp_reg_size;
	uint32_t i;

	rev_id = platform_get_qmp_rev();

	/* Sequence as per HPG */
	writel(0x01, QMP_PHY_BASE + PCIE_USB3_PHY_POWER_DOWN_CONTROL);

	if (rev_id >= 0x20000000)
	{
		qmp_reg_size = sizeof(qmp_settings_rev2) / sizeof(struct qmp_reg);
		for (i = 0 ; i < qmp_reg_size; i++)
			writel(qmp_settings_rev2[i].val, QMP_PHY_BASE + qmp_settings_rev2[i].off);

		if (target_override_pll())
		{
			qmp_reg_size = sizeof(qmp_override_pll_rev2) / sizeof(struct qmp_reg);
			for (i = 0 ; i < qmp_reg_size; i++)
				writel(qmp_override_pll_rev2[i].val, QMP_PHY_BASE + qmp_override_pll_rev2[i].off);
		}
	}
	else
	{
		writel(0x08, QMP_PHY_BASE + QSERDES_COM_SYSCLK_EN_SEL_TXBAND);

		if (target_override_pll())
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
		if (target_override_pll())
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

	while ((readl(QMP_PHY_BASE + phy_status) & PHYSTATUS))
	{
		udelay(1);
		timeout--;
		if (!timeout)
		{
			dprintf(CRITICAL, "QMP phy initialization failed, fallback to HighSpeed only mode\n");
			hsonly_mode = true;
			return;
		}
	}

	clock_bumpup_pipe3_clk();
}

EFI_STATUS
EFIAPI
AcquireEfiProtocols(VOID)
{
  EFI_STATUS Status;
  Status = gBS->LocateProtocol(
      &gQcomGpioTlmmProtocolGuid, NULL, (VOID **)&mTlmmProtocol);
  if (EFI_ERROR(Status))
    goto exit;

  Status =
      gBS->LocateProtocol(&gQcomRpmProtocolGuid, NULL, (VOID **)&mRpmProtocol);
  if (EFI_ERROR(Status))
    goto exit;

  Status = gBS->LocateProtocol(
      &gQcomPm8x41ProtocolGuid, NULL, (VOID **)&mPmicProtocol);
  if (EFI_ERROR(Status))
    goto exit;

  Status = gBS->LocateProtocol(
      &gQcomBoardProtocolGuid, NULL, (VOID **)&mBoardProtocol);
  if (EFI_ERROR(Status))
    goto exit;

  Status = gBS->LocateProtocol(
    &gQcomClockProtocolGuid, NULL, (VOID**) &mClockProtocol);
  if (EFI_ERROR(Status))
    goto exit;

  mBoardId = mBoardProtocol->board_platform_id();

exit:
  return Status;
}

EFI_STATUS
EFIAPI
Usb3PhyDxeEntry(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS status;

    DEBUG((EFI_D_INFO | EFI_D_LOAD, "-> Usb3PhyDxeEntry\n"));

    DEBUG((EFI_D_INFO | EFI_D_LOAD, "-> AcquireEfiProtocols\n"));
    status = AcquireEfiProtocols();
    if (EFI_ERROR(status)) goto exit;
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "<- AcquireEfiProtocols\n"));

    DEBUG((EFI_D_INFO | EFI_D_LOAD, "-> usb30_qmp_phy_reset\n"));
    usb30_qmp_phy_reset();
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "<- usb30_qmp_phy_reset\n"));

    DEBUG((EFI_D_INFO | EFI_D_LOAD, "-> usb30_qmp_phy_init\n"));
    usb30_qmp_phy_init();
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "<- usb30_qmp_phy_init\n"));

exit:
	ASSERT(hsonly_mode == false);
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "<- Usb3PhyDxeEntry 0x%x\n", status));
    return status;
}
