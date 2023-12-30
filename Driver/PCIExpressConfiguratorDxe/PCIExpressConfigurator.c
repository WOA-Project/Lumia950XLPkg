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

#include <Chipset/PCIExpress.h>
#include <Platform/iomap.h>

#include <Protocol/QcomBoard.h>
#include <Protocol/QcomGpioTlmm.h>
#include <Protocol/QcomPm8x41.h>
#include <Protocol/QcomRpm.h>

#include <IndustryStandard/Pci22.h>

#include "PCIeDefinition.h"

#define ENABLE_QCOM_RPM

QCOM_GPIO_TLMM_PROTOCOL *mTlmmProtocol;
#ifdef ENABLE_QCOM_RPM
QCOM_RPM_PROTOCOL *mRpmProtocol;
#endif
QCOM_PM8X41_PROTOCOL *mPmicProtocol;
QCOM_BOARD_PROTOCOL * mBoardProtocol;

uint32_t mBoardId = 0;

EFI_STATUS
EFIAPI
AcquireEfiProtocols(VOID)
{
  EFI_STATUS Status;
  Status = gBS->LocateProtocol(
      &gQcomGpioTlmmProtocolGuid, NULL, (VOID **)&mTlmmProtocol);
  if (EFI_ERROR(Status))
    goto exit;

#ifdef ENABLE_QCOM_RPM
  Status =
      gBS->LocateProtocol(&gQcomRpmProtocolGuid, NULL, (VOID **)&mRpmProtocol);
  if (EFI_ERROR(Status))
    goto exit;
#endif

  Status = gBS->LocateProtocol(
      &gQcomPm8x41ProtocolGuid, NULL, (VOID **)&mPmicProtocol);
  if (EFI_ERROR(Status))
    goto exit;

  Status = gBS->LocateProtocol(
      &gQcomBoardProtocolGuid, NULL, (VOID **)&mBoardProtocol);
  if (EFI_ERROR(Status))
    goto exit;

  mBoardId = mBoardProtocol->board_platform_id();

exit:
  return Status;
}

EFI_STATUS
EFIAPI
VerifyPlatform(VOID)
{
  if (mBoardId != MSM8994 && mBoardId != MSM8992 && mBoardId != APQ8094) {
    DEBUG(
        (EFI_D_ERROR | EFI_D_WARN,
         "Target platform is not MSM8992 or MSM8994/APQ8094. PCIe init skipped \n"));
    ASSERT(FALSE);
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
EnableClocksMsm8994(VOID)
{
  EFI_STATUS Status = EFI_SUCCESS;
  BOOLEAN initializePCIe1 = mBoardId == MSM8994 || mBoardId == APQ8094;

  switch (mBoardId)
  {
  case MSM8992:
    // HWIO_GCC_PCIE_0_PIPE_CBCR_ADDR
    MmioWrite32(0xFC401B14, 0);
    // HWIO_GCC_PCIE_0_PIPE_CMD_RCGR_ADDR
    MmioWrite32(0xFC401B18, 0);
    break;
  case MSM8994:
    // HWIO_GCC_PCIE_1_PIPE_CBCR_ADDR
    MmioWrite32(0xFC401B94, 0);
    // HWIO_GCC_PCIE_1_PIPE_CMD_RCGR_ADDR
    MmioWrite32(0xFC401B98, 0);
    break;
  case APQ8094:
    // HWIO_GCC_PCIE_0_PIPE_CBCR_ADDR
    MmioWrite32(0xFC401B14, 0);
    // HWIO_GCC_PCIE_0_PIPE_CMD_RCGR_ADDR
    MmioWrite32(0xFC401B18, 0);
    // HWIO_GCC_PCIE_1_PIPE_CBCR_ADDR
    MmioWrite32(0xFC401B94, 0);
    // HWIO_GCC_PCIE_1_PIPE_CMD_RCGR_ADDR
    MmioWrite32(0xFC401B98, 0);
    break;
  }

  // Clocks & LDOs
  // Power and Clock
  // GDSC & regulator
  // LDO12 (1.8V), LDO28 (0.9V)
#ifdef ENABLE_QCOM_RPM
  Status = mRpmProtocol->rpm_ldo_pipe_enable();
  if (EFI_ERROR(Status))
    goto exit;
  gBS->Stall(1000);
#endif

  // GDSC
  gdsc_pcie0_enable();
  if (initializePCIe1) gdsc_pcie1_enable();
  // pcie_1_ref_clk_src
#ifdef ENABLE_QCOM_RPM
  rpm_smd_ln_bb_clk_enable();
#endif
  // pcie_1_aux_clk
  pcie_0_aux_clk_set_rate_and_enable();
  if (initializePCIe1) pcie_1_aux_clk_set_rate_and_enable();
  // pcie_1_cfg_ahb_clk
  pcie_0_cfg_ahb_clk_enable();
  if (initializePCIe1) pcie_1_cfg_ahb_clk_enable();
  // pcie_1_mstr_axi_clk
  pcie_0_mstr_axi_clk_enable();
  if (initializePCIe1) pcie_1_mstr_axi_clk_enable();
  // pcie_1_slv_axi_clk
  pcie_0_slv_axi_clk_enable();
  if (initializePCIe1) pcie_1_slv_axi_clk_enable();
  // pcie_1_phy_ldo
  pcie_0_phy_ldo_enable();
  if (initializePCIe1) pcie_1_phy_ldo_enable();
  // pcie_phy_1_reset
  pcie_phy_0_reset_enable();
  if (initializePCIe1) pcie_phy_1_reset_enable();
  // Memory fence
  MemoryFence();

  exit:
  return Status;
}

EFI_STATUS
EFIAPI
ConfigurePCIeAndCnssGpio(VOID)
{
  // GPIO 35 PERST#, func Generic I/O (0), Dir Out, No Pull (0), Drive 2mA (0),
  // assert -> deassert
  mTlmmProtocol->SetFunction(35, 0);
  mTlmmProtocol->SetDriveStrength(35, 2);
  mTlmmProtocol->SetPull(35, GPIO_PULL_NONE);
  mTlmmProtocol->DirectionOutput(35, 1);
  mTlmmProtocol->Set(35, 0);

  // DB: GPIO 53 PERST#, func Generic I/O (0), Dir Out, No Pull (0), Drive 2mA (0),
  // assert -> deassert
  if (mBoardId == APQ8094)
  {
    mTlmmProtocol->SetFunction(53, 0);
    mTlmmProtocol->SetDriveStrength(53, 2);
    mTlmmProtocol->SetPull(53, GPIO_PULL_NONE);
    mTlmmProtocol->DirectionOutput(53, 1);
    mTlmmProtocol->Set(53, 0);
  }

  // GPIO 1, func Generic I/O, dir Out, No Pull, Drive 2mA, keep assert
  mTlmmProtocol->SetFunction(1, 0);
  mTlmmProtocol->SetDriveStrength(1, 2);
  mTlmmProtocol->SetPull(1, GPIO_PULL_NONE);
  mTlmmProtocol->DirectionOutput(1, 1);
  mTlmmProtocol->Set(1, 1);

  // GPIO 112 for CNSS bootstrap: generic I/O, dir out, pull up, drive 2mA, keep
  // assert
  mTlmmProtocol->SetFunction(112, 0);
  mTlmmProtocol->SetDriveStrength(112, 2);
  mTlmmProtocol->SetPull(112, GPIO_PULL_UP);
  mTlmmProtocol->DirectionOutput(112, 1);
  mTlmmProtocol->Set(112, 1);

  // WLAN_LDO_3V_CTRL (PM8994 GPIO9) configure as out
  // Skip?

  // GPIO_19 BT (PM8994 GPIO19) configure as out
  // Skip?

  // GPIO 113 for WLAN_EN: generic I/O, dir out, no pull, drive 2mA, assert ->
  // deassert
  mTlmmProtocol->SetFunction(113, 0);
  mTlmmProtocol->SetDriveStrength(113, 2);
  mTlmmProtocol->SetPull(113, GPIO_PULL_NONE);
  mTlmmProtocol->DirectionOutput(113, 1);
  mTlmmProtocol->Set(113, 0);

  // GPIO 50 Unknown: generic I/O, dir out, no pull, drive 2mA, assert ->
  // deassert
  mTlmmProtocol->SetFunction(50, 0);
  mTlmmProtocol->SetDriveStrength(50, 2);
  mTlmmProtocol->SetPull(50, GPIO_PULL_NONE);
  mTlmmProtocol->DirectionOutput(50, 1);
  mTlmmProtocol->Set(50, 0);

  // Assert GPIO 113, hold short
  mTlmmProtocol->Set(113, 1);
  gBS->Stall(1000);

  return EFI_SUCCESS;
}

static inline void InitializePCIePHYWithBase(UINTN MsmPciePhyBase)
{
  if (mBoardId == APQ8094)
  {
    MmioWrite32(MsmPciePhyBase + PCIE_PHY_POWER_DOWN_CONTROL, 0x03);

    MmioWrite32(MsmPciePhyBase + QSERDES_COM_SYSCLK_EN_SEL_TXBAND, 0x08);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_DEC_START1, 0x82);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_DEC_START2, 0x03);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_DIV_FRAC_START1, 0xD5);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_DIV_FRAC_START2, 0xAA);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_DIV_FRAC_START3, 0x4D);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLLLOCK_CMP_EN, 0x03);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLLLOCK_CMP1, 0x06);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLLLOCK_CMP2, 0x1A);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_CRCTRL, 0x7C);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_CP_SETI, 0x1F);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_IP_SETP, 0x12);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_CP_SETP, 0x0F);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_IP_SETI, 0x01);

    MmioWrite32(MsmPciePhyBase + QSERDES_COM_IE_TRIM, 0x0F);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_IP_TRIM, 0x0F);

    MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_CNTRL, 0x46);

    /* CDR Settings */
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_CDR_CONTROL1, 0xF4);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_CDR_CONTROL_HALF, 0x2C);

    MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_VCOTAIL_EN, 0xE1);

    /* Calibration Settings */
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_RESETSM_CNTRL, 0x91);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_RESETSM_CNTRL2, 0x07);

    /* Additional writes */
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_RES_CODE_START_SEG1, 0x20);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_RES_CODE_CAL_CSR, 0x77);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_RES_TRIM_CONTROL, 0x15);
    MmioWrite32(MsmPciePhyBase + QSERDES_TX_RCV_DETECT_LVL, 0x03);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_UCDR_FO_GAIN, 0x09);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_UCDR_SO_GAIN, 0x04);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_UCDR_SO_SATURATION_AND_ENABLE,
          0x49);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_EQ_GAIN1_LSB, 0xFF);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_EQ_GAIN1_MSB, 0x1F);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_EQ_GAIN2_LSB, 0xFF);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_EQ_GAIN2_MSB, 0x00);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_EQU_ADAPTOR_CNTRL2, 0x1E);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_EQ_OFFSET_ADAPTOR_CNTRL1,
          0x67);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_OFFSET_ADAPTOR_CNTRL2, 0x80);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_SIGDET_ENABLES, 0x40);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_SIGDET_CNTRL, 0xB0);
    MmioWrite32(MsmPciePhyBase + QSERDES_RX_SIGDET_DEGLITCH_CNTRL, 0x06);
    MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_RXTXEPCLK_EN, 0x10);
    MmioWrite32(MsmPciePhyBase + PCIE_PHY_ENDPOINT_REFCLK_DRIVE, 0x10);
    MmioWrite32(MsmPciePhyBase + PCIE_PHY_POWER_STATE_CONFIG1, 0xA3);
    MmioWrite32(MsmPciePhyBase + PCIE_PHY_POWER_STATE_CONFIG2, 0x4B);
    MmioWrite32(MsmPciePhyBase + PCIE_PHY_RX_IDLE_DTCT_CNTRL, 0x4D);

    MmioWrite32(MsmPciePhyBase + PCIE_PHY_SW_RESET, 0x00);
    MmioWrite32(MsmPciePhyBase + PCIE_PHY_START, 0x03);

    return;
  }

  MmioWrite32(MsmPciePhyBase + PCIE_PHY_POWER_DOWN_CONTROL, 0x3);

  MmioWrite32(MsmPciePhyBase + QSERDES_COM_SYSCLK_EN_SEL_TXBAND, 0x8);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_DEC_START1, 0x82);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_DEC_START2, 0x3);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_DIV_FRAC_START1, 0x0D5);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_DIV_FRAC_START2, 0x0AA);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_DIV_FRAC_START3, 0x4D);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLLLOCK_CMP_EN, 0x7);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLLLOCK_CMP1, 0x41);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLLLOCK_CMP2, 0x3);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_CRCTRL, 0x7c);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_CP_SETI, 0x7);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_IP_SETP, 0x1F);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_CP_SETP, 0xF);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_IP_SETI, 0x1);

  MmioWrite32(MsmPciePhyBase + QSERDES_COM_IE_TRIM, 0xF);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_IP_TRIM, 0xF);

  MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_CNTRL, 0x46);

  MmioWrite32(MsmPciePhyBase + QSERDES_RX_CDR_CONTROL1, 0xF5);
  MmioWrite32(MsmPciePhyBase + QSERDES_RX_CDR_CONTROL_HALF, 0x2C);

  MmioWrite32(MsmPciePhyBase + QSERDES_COM_RESETSM_CNTRL, 0x91);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_RESETSM_CNTRL2, 0x7);

  MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_VCOTAIL_EN, 0x0E1);

  MmioWrite32(MsmPciePhyBase + QSERDES_COM_RES_CODE_START_SEG1, 0x24);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_RES_CODE_CAL_CSR, 0x77);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_RES_TRIM_CONTROL, 0x15);
  MmioWrite32(MsmPciePhyBase + QSERDES_TX_RCV_DETECT_LVL, 0x3);

  MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_EQ_GAIN1_LSB, 0xFF);
  MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_EQ_GAIN1_MSB, 0x7);
  MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_EQ_GAIN2_LSB, 0xFF);
  MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_EQ_GAIN2_MSB, 0x0);
  MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_EQU_ADAPTOR_CNTRL2, 0x1E);
  MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_EQ_OFFSET_ADAPTOR_CNTRL1, 0x67);
  MmioWrite32(MsmPciePhyBase + QSERDES_RX_RX_OFFSET_ADAPTOR_CNTRL2, 0x80);
  MmioWrite32(MsmPciePhyBase + QSERDES_RX_SIGDET_ENABLES, 0x40);
  MmioWrite32(MsmPciePhyBase + QSERDES_RX_SIGDET_CNTRL, 0x70);
  MmioWrite32(MsmPciePhyBase + QSERDES_RX_SIGDET_DEGLITCH_CNTRL, 0x0C);

  MmioWrite32(MsmPciePhyBase + QSERDES_COM_SSC_EN_CENTER, 0x1);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_SSC_ADJ_PER1, 0x2);
  MmioWrite32(MsmPciePhyBase + QSERDES_COM_SSC_PER1, 0x31);
  MmioWrite32(MsmPciePhyBase + QPHY_FLL_CNTRL1, 0x1);
  MmioWrite32(MsmPciePhyBase + QPHY_FLL_CNTRL2, 0x19);
  MmioWrite32(MsmPciePhyBase + QPHY_FLL_CNT_VAL_L, 0x19);

  MmioWrite32(MsmPciePhyBase + QSERDES_COM_PLL_RXTXEPCLK_EN, 0x10);
  MmioWrite32(MsmPciePhyBase + PCIE_PHY_ENDPOINT_REFCLK_DRIVE, 0x10);
  MmioWrite32(MsmPciePhyBase + PCIE_PHY_POWER_STATE_CONFIG1, 0x23);
  MmioWrite32(MsmPciePhyBase + PCIE_PHY_POWER_STATE_CONFIG2, 0x4B);
  MmioWrite32(MsmPciePhyBase + PCIE_PHY_RX_IDLE_DTCT_CNTRL, 0x4D);

  MmioWrite32(MsmPciePhyBase + PCIE_PHY_SW_RESET, 0x0);
  MmioWrite32(MsmPciePhyBase + PCIE_PHY_START, 0x3);
}

static inline void msm_pcie_write_mask(UINTN addr, uint32_t clear_mask, uint32_t set_mask)
{
	uint32_t val;

	val = (readl_relaxed(addr) & ~clear_mask) | set_mask;
	writel_relaxed(val, addr);
	dmb();  /* ensure data is written to hardware register */
}

EFI_STATUS
EFIAPI
InitializePciePHY(VOID)
{
  switch (mBoardId)
  {
  case MSM8992:
    // GCC clks (GCC_PCIE_0_BCR)
    MmioWrite32(0xFC401AC0, 0x1);
    gBS->Stall(2000);
    MmioWrite32(0xFC401AC0, 0x0);
    // PHY base
    InitializePCIePHYWithBase(0xfc526000);
    break;
  case MSM8994:
    // GCC clks (GCC_PCIE_1_BCR)
    MmioWrite32(0xFC401B40, 0x1);
    gBS->Stall(2000);
    MmioWrite32(0xFC401B40, 0x0);
    // PHY base
    InitializePCIePHYWithBase(0xfc52e000);
    break;
  case APQ8094:
    // DB-specific: set some PARF flags
    msm_pcie_write_mask(0xfc520000 + PCIE20_PARF_PHY_CTRL, BIT(0), 0);
    msm_pcie_write_mask(0xfc528000 + PCIE20_PARF_PHY_CTRL, BIT(0), 0);

    // GCC clks (GCC_PCIE_0_BCR)
    MmioWrite32(0xFC401AC0, 0x1);
    gBS->Stall(2000);
    MmioWrite32(0xFC401AC0, 0x0);
    // PHY base
    InitializePCIePHYWithBase(0xfc526000);

    // GCC clks (GCC_PCIE_1_BCR)
    MmioWrite32(0xFC401B40, 0x1);
    gBS->Stall(2000);
    MmioWrite32(0xFC401B40, 0x0);
    // PHY base
    InitializePCIePHYWithBase(0xfc52e000);
    break;
  }

  return EFI_SUCCESS;
}

static inline int WaitForPCIePHYReadyWithBase(UINTN MsmPciePhyBase)
{
  // Wait for PHY to get ready
  UINTN status = MmioRead32(MsmPciePhyBase + PCIE_PHY_PCS_STATUS);
  UINTN cnt = 0;
  UINTN sec = 0;

  while (status & 0x40)
  {
    gBS->Stall(5);
    cnt += 5;

    if (cnt > 1000000) {
      cnt = 0;
      sec++;
      DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR,
        "Waiting for PCIe PHY to get ready... BASE: 0x%x, STATUS: 0x%x\n", MsmPciePhyBase, status));

      if (sec > 10) {
        DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR,
          "PHY didn't come up! BASE: 0x%x, STATUS: 0x%x\n", MsmPciePhyBase, status));
        return -1;
      }
    }
    status = MmioRead32(MsmPciePhyBase + PCIE_PHY_PCS_STATUS);
  }

  return 0;
}

EFI_STATUS
EFIAPI
WaitForPCIePHYReady(VOID)
{
  int ret = 0;

  switch (mBoardId)
  {
  case MSM8992:
    ret = WaitForPCIePHYReadyWithBase(0xfc526000);
    if (ret < 0) return EFI_TIMEOUT;
    break;
  case MSM8994:
    ret = WaitForPCIePHYReadyWithBase(0xfc52e000);
    if (ret < 0) return EFI_TIMEOUT;
    break;
  case APQ8094:
    ret = WaitForPCIePHYReadyWithBase(0xfc526000);
    ret = WaitForPCIePHYReadyWithBase(0xfc52e000);
    if (ret < 0) return EFI_TIMEOUT;
    break;
  }

  // Assert GPIO 35 PERST#
  mTlmmProtocol->SetFunction(35, 0);
  mTlmmProtocol->SetDriveStrength(35, 2);
  mTlmmProtocol->SetPull(35, GPIO_PULL_NONE);
  mTlmmProtocol->DirectionOutput(35, 1);
  mTlmmProtocol->Set(35, 1);

  // Assert GPIO 53 PERST#
  if (mBoardId == APQ8094)
  {
    mTlmmProtocol->SetFunction(53, 0);
    mTlmmProtocol->SetDriveStrength(53, 2);
    mTlmmProtocol->SetPull(53, GPIO_PULL_NONE);
    mTlmmProtocol->DirectionOutput(53, 1);
    mTlmmProtocol->Set(53, 1);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RpmTurnOnLdo30(VOID)
{
#ifdef ENABLE_QCOM_RPM
  return mRpmProtocol->rpm_ldo30_enable();
#else
  return EFI_SUCCESS;
#endif
}

EFI_STATUS
EFIAPI
SetPipeClock(VOID)
{
  // pcie_0_pipe_clk
  pcie_0_pipe_clk_set_rate_enable();
  MemoryFence();
  gBS->Stall(REFCLK_STABILIZATION_DELAY_US_MIN);

  // pcie_1_pipe_clk
  pcie_1_pipe_clk_set_rate_enable();
  MemoryFence();
  gBS->Stall(REFCLK_STABILIZATION_DELAY_US_MIN);

  return EFI_SUCCESS;
}

static inline void EnableLinkWithAddr(UINTN ParfBase, UINTN DmCoreBase)
{
  UINT32 Val = 0;

  // PCIE20_PARF_DEVICE_TYPE: this is RC
  MmioWrite32(ParfBase + 0x1000, 0x4);
  // PCIE20_ELBI_SYS_CTRL
  MmioWrite32(DmCoreBase + 0xf24, 0x1);
  // What's this? A guess is PCIE20_CAP_LINKCTRLSTATUS2
  MmioWrite32(DmCoreBase + 0x88, 0x1000000);
  gBS->Stall(1000);

  // PCIE20_COMMAND_STATUS
  Val = MmioRead32(DmCoreBase + 0x4);
  MmioWrite32(DmCoreBase + 0x4, Val | 0x46);

  // PCIE20_LINK_CONTROL2_LINK_STATUS2
  Val = MmioRead32(DmCoreBase + 0xa0);
  MmioWrite32(DmCoreBase + 0xa0, (Val & 0xFFFFFFF0) + 1);

  // Enable Link Training State Machine
  MmioWrite32(ParfBase + PCIE20_PARF_LTSSM, 0x100);

  // Check link (PCIE20_ELBI_SYS_STTS)
  while ((MmioRead32(DmCoreBase + 0xf28) & 0x400) != 0x400)
    gBS->Stall(1000);
}

EFI_STATUS
EFIAPI
EnableLink(VOID)
{
  
  UINTN  ParfBase   = 0;
  UINTN  DmCoreBase = 0;

  switch (mBoardId)
  {
    case MSM8992:
      EnableLinkWithAddr(0xfc520000, 0xff000000);
      break;
    case MSM8994:
      EnableLinkWithAddr(0xfc528000, 0xf8800000);
      break;
    case APQ8094:
      EnableLinkWithAddr(0xfc520000, 0xff000000);
      EnableLinkWithAddr(0xfc528000, 0xf8800000);
      break;
  }

  return EFI_SUCCESS;
}

static inline void ConfigureDmCoreWithBase(UINTN DmCoreBase)
{
  UINT32 Val = 0;

  // PCIE20_MISC_CONTROL_1
  MmioWrite32(DmCoreBase + 0x8bc, 0x1);
  // PCIE20_LINK_CAPABILITIES
  Val = MmioRead32(DmCoreBase + 0x7c);
  MmioWrite32(DmCoreBase + 0x7c, (Val & 0xFFFFFBFF) | 0x400800);
  // PCIE20_MISC_CONTROL_1
  MmioWrite32(DmCoreBase + 0x8bc, 0x0);
  // PCIE20_L1SUB_CAPABILITY
  Val = MmioRead32(DmCoreBase + 0x154);
  MmioWrite32(DmCoreBase + 0x154, Val | 0xF);
  // PCIE20_L1SUB_CONTROL1
  Val = MmioRead32(DmCoreBase + 0x158);
  MmioWrite32(DmCoreBase + 0x158, Val | 0xF);
  // PCIE20_DEVICE_CONTROL2_STATUS2
  Val = MmioRead32(DmCoreBase + 0x98);
  MmioWrite32(DmCoreBase + 0x98, 0x400);
}

EFI_STATUS
EFIAPI
ConfigDmCore(VOID)
{
  switch (mBoardId)
  {
  case MSM8992:
    ConfigureDmCoreWithBase(0xff000000);
    break;
  case MSM8994:
    ConfigureDmCoreWithBase(0xf8800000);
    break;
  case APQ8094:
    ConfigureDmCoreWithBase(0xff000000);
    ConfigureDmCoreWithBase(0xf8800000);
    break;
  }

  // GPIO 36, in, function 2, 2mA drive, no pull (clkreq)
  mTlmmProtocol->SetFunction(36, 2);
  mTlmmProtocol->SetDriveStrength(36, 2);
  mTlmmProtocol->SetPull(36, GPIO_PULL_NONE);
  mTlmmProtocol->DirectionInput(36);

  // GPIO 54, in, function 2, 2mA drive, no pull (clkreq)
  if (mBoardId == APQ8094)
  {
    mTlmmProtocol->SetFunction(54, 2);
    mTlmmProtocol->SetDriveStrength(54, 2);
    mTlmmProtocol->SetPull(54, GPIO_PULL_NONE);
    mTlmmProtocol->DirectionInput(54);
  }

  return EFI_SUCCESS;
}

static inline void ConfigSpaceWithBase(UINTN DmCoreBase)
{
  UINT32 i          = 1;
  UINT32 j          = 4;
  UINT32 k          = 0;
  UINT32 Addr       = 0;
  UINT32 Val        = 0;

  while (TRUE) {
    // DM_CORE, base varies
    // PCIE20_PLR_IATU_VIEWPORT
    MmioWrite32(DmCoreBase + 0x900, i);
    // PCIE20_PLR_IATU_CTRL1
    MmioWrite32(DmCoreBase + 0x904, j);
    // PCIE20_PLR_IATU_CTRL2
    MmioWrite32(DmCoreBase + 0x908, 0x80000000);
    // PCIE20_PLR_IATU_VIEWPORT
    Addr = DmCoreBase + 0x900 + (k << 20) + 0x100000;
    // PCIE20_PLR_IATU_LBAR
    MmioWrite32(DmCoreBase + 0x90c, Addr);
    // PCIE20_PLR_IATU_UBAR
    MmioWrite32(DmCoreBase + 0x910, 0);
    // PCIE20_PLR_IATU_LAR
    MmioWrite32(DmCoreBase + 0x914, Addr);
    // PCIE20_PLR_IATU_LTAR
    MmioWrite32(DmCoreBase + 0x918, ((k + 1) << 24));
    // PCIE20_PLR_IATU_UTAR
    MmioWrite32(DmCoreBase + 0x91c, 0);
    // What's this? PCIE20_BUSNUMBERS?
    MmioWrite32(DmCoreBase + 0x18, 0x30100);
    // What's this?
    Val = MmioRead32(Addr + 0x188);
    MmioWrite32(Addr + 0x188, Val | 0xF);
    // PCIE20_DEVICE_CONTROL2_STATUS2
    Val = MmioRead32(Addr + 0x98);
    MmioWrite32(Addr + 0x98, Val | 0x400);
    // Three bars
    ++i;
    if (++k < 2)
      break;
    if (k)
      j = 5;
  }
}

EFI_STATUS
EFIAPI
ConfigSpace(VOID)
{
  switch (mBoardId)
  {
    case MSM8992:
      ConfigSpaceWithBase(0xff000000);
      break;
    case MSM8994:
      ConfigSpaceWithBase(0xf8800000);
      break;
    case APQ8094:
      ConfigSpaceWithBase(0xff000000);
      ConfigSpaceWithBase(0xf8800000);
      break;
  }

  return EFI_SUCCESS;
}

static inline void ConfigBARWithBase(UINTN DmCoreBase)
{
  UINT32  i          = 0;
  UINT32  j          = 0;
  UINT32  k          = 16;
  UINT32  Val        = 0;
  BOOLEAN SetI       = FALSE;

  UINT32 Addr = DmCoreBase + 0x100000;
  UINT32 Adr1 = DmCoreBase + 0x400000;

  do {
    MmioWrite32(Addr + k, 0xFFFFFFFF);
    Val = MmioRead32(Addr + k);
    if (Val && !(Val << 31)) {
      if (Val & 8) {
        SetI = TRUE;
      }
      else {
        SetI = FALSE;
      }

      if (((Val >> 1) & 3) != 2 || (k += 4, MmioWrite32(Addr + k, 0xFFFFFFFF),
                                    MmioRead32(Addr + k) == -1)) {
        if (SetI) {
          i = i - (Val & 0xFFFFFFF0);
        }
        else {
          j = j - (Val & 0xFFFFFFF0);
        }
      }
    }
    k += 4;
  } while (k <= 0x24);

  MmioWrite32(DmCoreBase + 0x20, (((Adr1 + j) >> 16) << 16) | (Adr1 >> 16));
  MmioWrite32(
      DmCoreBase + 0x24,
      ((Adr1 + j + i) & 0xFFFF0000) | ((Adr1 + j + i) >> 16));
}

EFI_STATUS
EFIAPI
FinishingUp(VOID)
{
  UINT32  i          = 0;
  UINT32  j          = 0;
  UINTN   DmCoreBase = 0;
  UINT32  k          = 16;
  UINT32  Val        = 0;
  BOOLEAN SetI       = FALSE;

  switch (mBoardId)
  {
  case MSM8992:
    ConfigBARWithBase(0xff000000);
    break;
  case MSM8994:
    ConfigBARWithBase(0xf8800000);
    break;
  case APQ8094:
    ConfigBARWithBase(0xff000000);
    ConfigBARWithBase(0xf8800000);
    break;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PCIExpressConfiguratorEntry(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry ->\n"));

  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry -> AcquireEfiProtocols\n"));
  Status = AcquireEfiProtocols();
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry <- AcquireEfiProtocols\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry -> VerifyPlatform\n"));
  Status = VerifyPlatform();
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry <- VerifyPlatform\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry -> EnableClocksMsm8994\n"));
  Status = EnableClocksMsm8994();
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry <- EnableClocksMsm8994\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry -> ConfigurePCIeAndCnssGpio\n"));
  Status = ConfigurePCIeAndCnssGpio();
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry <- ConfigurePCIeAndCnssGpio\n"));
  if (EFI_ERROR(Status))
    goto exit;

  // TODO: Call msm_pcie_restore_sec_config to restore security config
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry -> InitializePciePHY\n"));
  Status = InitializePciePHY();
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry <- InitializePciePHY\n"));
  if (EFI_ERROR(Status))
    goto exit;

#ifdef ENABLE_QCOM_RPM
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry -> RpmTurnOnLdo30\n"));
  Status = RpmTurnOnLdo30();
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry <- RpmTurnOnLdo30\n"));
  if (EFI_ERROR(Status))
    goto exit;
#endif

  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry -> SetPipeClock\n"));
  Status = SetPipeClock();
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry <- SetPipeClock\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry -> WaitForPCIePHYReady\n"));
  Status = WaitForPCIePHYReady();
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry <- WaitForPCIePHYReady\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry -> EnableLink\n"));
  Status = EnableLink();
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry <- EnableLink\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry -> ConfigDmCore\n"));
  Status = ConfigDmCore();
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry <- ConfigDmCore\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry -> ConfigSpace\n"));
  Status = ConfigSpace();
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry <- ConfigSpace\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry -> FinishingUp\n"));
  Status = FinishingUp();
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry <- FinishingUp\n"));
  if (EFI_ERROR(Status))
    goto exit;

  Status = gBS->InstallProtocolInterface(
      &ImageHandle, &gQcomMsmPCIExpressInitProtocolGuid, EFI_NATIVE_INTERFACE,
      &ImageHandle);

exit:
  DEBUG((EFI_D_LOAD | EFI_D_INFO | EFI_D_ERROR, "PCIExpressConfiguratorEntry <-\n"));
  return Status;
}
