#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/ArmLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <GplLibrary/clock-rpm-msm8994.h>
#include <GplLibrary/gdsc-msm8994.h>

#include <Chipset/PCIExpress.h>
#include <Platform/iomap.h>

#include <Protocol/QcomBoard.h>
#include <Protocol/QcomGpioTlmm.h>
#include <Protocol/QcomPm8x41.h>
#include <Protocol/QcomRpm.h>
#include <Protocol/QcomClock.h>

#include <IndustryStandard/Pci22.h>

#include "PCIeDefinition.h"

#define ENABLE_QCOM_RPM

struct pcie_clocks {
  struct clk *pipe_clk;
  struct clk *aux_clk;
  struct clk *cfg_ahb_clk;
  struct clk *mstr_axi_clk;
  struct clk *slv_axi_clk;
  struct clk *phy_ldo;
  struct clk *phy_reset;
};

static QCOM_GPIO_TLMM_PROTOCOL *mTlmmProtocol;
#ifdef ENABLE_QCOM_RPM
static QCOM_RPM_PROTOCOL *mRpmProtocol;
#endif
static QCOM_PM8X41_PROTOCOL *mPmicProtocol;
static QCOM_BOARD_PROTOCOL * mBoardProtocol;
static QCOM_CLOCK_PROTOCOL *mClockProtocol;

static uint32_t mBoardId = 0;
static BOOLEAN initializePCIe0 = FALSE;
static BOOLEAN initializePCIe1 = FALSE;
static BOOLEAN isPCIe0LinkUp = FALSE;
static BOOLEAN isPCIe1LinkUp = FALSE;
static struct pcie_clocks mPCIeClocks[2] = { {0}, {0}, };

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

  Status = gBS->LocateProtocol(
    &gQcomClockProtocolGuid, NULL, (VOID**) &mClockProtocol);
  if (EFI_ERROR(Status))
    goto exit;

  mBoardId = mBoardProtocol->board_platform_id();
  initializePCIe0 = mBoardId == MSM8992 || mBoardId == APQ8094;
  initializePCIe1 = mBoardId == MSM8994 || mBoardId == APQ8094;

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "Initialize PCIe0: %d, PCIe1: %d\n", initializePCIe0, initializePCIe1));
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

  if (initializePCIe0)
  {
    mPCIeClocks[0].pipe_clk = mClockProtocol->clk_get("pcie_0_pipe_clk");
    mPCIeClocks[0].aux_clk = mClockProtocol->clk_get("pcie_0_aux_clk");
    mPCIeClocks[0].cfg_ahb_clk = mClockProtocol->clk_get("pcie_0_cfg_ahb_clk");
    mPCIeClocks[0].mstr_axi_clk = mClockProtocol->clk_get("pcie_0_mstr_axi_clk");
    mPCIeClocks[0].slv_axi_clk = mClockProtocol->clk_get("pcie_0_slv_axi_clk");
    mPCIeClocks[0].phy_ldo = mClockProtocol->clk_get("pcie_0_phy_ldo");
    mPCIeClocks[0].phy_reset = mClockProtocol->clk_get("pcie_phy_0_reset");

    mClockProtocol->clk_reset(mPCIeClocks[0].pipe_clk, CLK_RESET_DEASSERT);
    mClockProtocol->clk_reset(mPCIeClocks[0].aux_clk, CLK_RESET_DEASSERT);
    mClockProtocol->clk_reset(mPCIeClocks[0].cfg_ahb_clk, CLK_RESET_DEASSERT);
    mClockProtocol->clk_reset(mPCIeClocks[0].mstr_axi_clk, CLK_RESET_DEASSERT);
    mClockProtocol->clk_reset(mPCIeClocks[0].slv_axi_clk, CLK_RESET_DEASSERT);
    mClockProtocol->clk_reset(mPCIeClocks[0].phy_ldo, CLK_RESET_DEASSERT);
    mClockProtocol->clk_reset(mPCIeClocks[0].phy_reset, CLK_RESET_DEASSERT);
  }

  if (initializePCIe1)
  {
    mPCIeClocks[1].pipe_clk = mClockProtocol->clk_get("pcie_1_pipe_clk");
    mPCIeClocks[1].aux_clk = mClockProtocol->clk_get("pcie_1_aux_clk");
    mPCIeClocks[1].cfg_ahb_clk = mClockProtocol->clk_get("pcie_1_cfg_ahb_clk");
    mPCIeClocks[1].mstr_axi_clk = mClockProtocol->clk_get("pcie_1_mstr_axi_clk");
    mPCIeClocks[1].slv_axi_clk = mClockProtocol->clk_get("pcie_1_slv_axi_clk");
    mPCIeClocks[1].phy_ldo = mClockProtocol->clk_get("pcie_1_phy_ldo");
    mPCIeClocks[1].phy_reset = mClockProtocol->clk_get("pcie_phy_1_reset");

    mClockProtocol->clk_reset(mPCIeClocks[1].pipe_clk, CLK_RESET_DEASSERT);
    mClockProtocol->clk_reset(mPCIeClocks[1].aux_clk, CLK_RESET_DEASSERT);
    mClockProtocol->clk_reset(mPCIeClocks[1].cfg_ahb_clk, CLK_RESET_DEASSERT);
    mClockProtocol->clk_reset(mPCIeClocks[1].mstr_axi_clk, CLK_RESET_DEASSERT);
    mClockProtocol->clk_reset(mPCIeClocks[1].slv_axi_clk, CLK_RESET_DEASSERT);
    mClockProtocol->clk_reset(mPCIeClocks[1].phy_ldo, CLK_RESET_DEASSERT);
    mClockProtocol->clk_reset(mPCIeClocks[1].phy_reset, CLK_RESET_DEASSERT);
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
  if (initializePCIe0) gdsc_pcie0_enable();
  if (initializePCIe1) gdsc_pcie1_enable();

  // pcie_1_ref_clk_src
#ifdef ENABLE_QCOM_RPM
  rpm_smd_ln_bb_clk_enable();
#endif

  // Auxiliary Clock
  if (initializePCIe0)
  {
    mClockProtocol->clk_enable(mPCIeClocks[0].aux_clk);
    mClockProtocol->clk_set_rate(mPCIeClocks[0].aux_clk, 1011000);
  }
  if (initializePCIe1)
  {
    mClockProtocol->clk_enable(mPCIeClocks[1].aux_clk);
    mClockProtocol->clk_set_rate(mPCIeClocks[1].aux_clk, 1011000);
  }

  // AHB Config Clock
  if (initializePCIe0) mClockProtocol->clk_enable(mPCIeClocks[0].cfg_ahb_clk);
  if (initializePCIe1) mClockProtocol->clk_enable(mPCIeClocks[1].cfg_ahb_clk);

  // AXI Memory Stream Clock
  if (initializePCIe0) mClockProtocol->clk_enable(mPCIeClocks[0].mstr_axi_clk);
  if (initializePCIe1) mClockProtocol->clk_enable(mPCIeClocks[1].mstr_axi_clk);

  // AXI Slave Clock
  if (initializePCIe0) mClockProtocol->clk_enable(mPCIeClocks[0].slv_axi_clk);
  if (initializePCIe1) mClockProtocol->clk_enable(mPCIeClocks[1].slv_axi_clk);

  // PHY LDO Clock
  if (initializePCIe0) mClockProtocol->clk_enable(mPCIeClocks[0].phy_ldo);
  if (initializePCIe1) mClockProtocol->clk_enable(mPCIeClocks[1].phy_ldo);

  // PHY Reset (?)
  if (initializePCIe0)
  {
    mClockProtocol->clk_reset(mPCIeClocks[0].phy_reset, CLK_RESET_ASSERT);
    udelay(100);
    mClockProtocol->clk_reset(mPCIeClocks[0].phy_reset, CLK_RESET_DEASSERT);
    mClockProtocol->clk_enable(mPCIeClocks[0].phy_reset);
  }
  if (initializePCIe1)
  {
    mClockProtocol->clk_reset(mPCIeClocks[1].phy_reset, CLK_RESET_ASSERT);
    udelay(100);
    mClockProtocol->clk_reset(mPCIeClocks[1].phy_reset, CLK_RESET_DEASSERT);
    mClockProtocol->clk_enable(mPCIeClocks[1].phy_reset);
  }

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
  if (initializePCIe0) InitializePCIePHYWithBase(0xfc526000);
  if (initializePCIe1) InitializePCIePHYWithBase(0xfc52e000);

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

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PHY is online! BASE: 0x%x, STATUS: 0x%x\n", MsmPciePhyBase, status));
  return 0;
}

EFI_STATUS
EFIAPI
WaitForPCIePHYReady(VOID)
{
  int ret = 0;

  if (initializePCIe0)
  {
    ret = WaitForPCIePHYReadyWithBase(0xfc526000);
    if (ret < 0) return EFI_TIMEOUT;
  }

  if (initializePCIe1)
  {
    ret = WaitForPCIePHYReadyWithBase(0xfc52e000);
    if (ret < 0) return EFI_TIMEOUT;
  }

  // DeAssert GPIO 35 PERST#
  mTlmmProtocol->SetFunction(35, 0);
  mTlmmProtocol->SetDriveStrength(35, 2);
  mTlmmProtocol->SetPull(35, GPIO_PULL_NONE);
  mTlmmProtocol->DirectionOutput(35, 1);
  mTlmmProtocol->Set(35, 1);

  // DeAssert GPIO 53 PERST#
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
  if (initializePCIe0)
  {
    mClockProtocol->clk_enable(mPCIeClocks[0].pipe_clk);
    mClockProtocol->clk_set_rate(mPCIeClocks[0].pipe_clk, 125000000);
    MemoryFence();
    gBS->Stall(REFCLK_STABILIZATION_DELAY_US_MIN);
  }

  if (initializePCIe1)
  {
    mClockProtocol->clk_enable(mPCIeClocks[1].pipe_clk);
    mClockProtocol->clk_set_rate(mPCIeClocks[1].pipe_clk, 125000000);
    MemoryFence();
    gBS->Stall(REFCLK_STABILIZATION_DELAY_US_MIN);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RestoreSecurityConfig(VOID)
{
  ARM_SMC_ARGS RestoreSecConfigArgs;

  if (mBoardId != APQ8094 && mBoardId != MSM8994) return EFI_SUCCESS;
  if (initializePCIe0)
  {
    ZeroMem(&RestoreSecConfigArgs, sizeof(RestoreSecConfigArgs));
    RestoreSecConfigArgs.Arg0 = 0x02000C02;
    RestoreSecConfigArgs.Arg1 = 2;
    RestoreSecConfigArgs.Arg2 = 11;
    ArmCallSmc(&RestoreSecConfigArgs);
  }
  if (initializePCIe1)
  {
    ZeroMem(&RestoreSecConfigArgs, sizeof(RestoreSecConfigArgs));
    RestoreSecConfigArgs.Arg0 = 0x02000C02;
    RestoreSecConfigArgs.Arg1 = 2;
    RestoreSecConfigArgs.Arg2 = 12;
    ArmCallSmc(&RestoreSecConfigArgs);
  }

  return EFI_SUCCESS;
}

static inline void EnableLinkWithAddr(UINTN ParfBase, UINTN DmCoreBase, BOOLEAN *pLinkUp)
{
  UINT32 Val = 0;
  UINT32 Cnt = 0;

  // PCIE20_PARF_DEVICE_TYPE: this is RC
  MmioWrite32(ParfBase + 0x1000, 0x4);
  // PCIE20_ELBI_SYS_CTRL (LTSSM_EN = 1)
  MmioWrite32(DmCoreBase + 0xf24, 0x1);
  // PCIE20_SLOT_CAS (DSC = 1)
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

  // Check link (PCIE20_ELBI_SYS_STTS), wait 100ms
  while ((MmioRead32(DmCoreBase + 0xf28) & 0x400) != 0x400)
  {
    gBS->Stall(1000);
    Cnt++;

    if (Cnt > 100)
    {
      DEBUG((EFI_D_LOAD | EFI_D_ERROR, "PCIe link doesn't come up in 100ms! BASE 0x%x\n", DmCoreBase));
      *pLinkUp = FALSE;
      return;
    }
  }

  *pLinkUp = TRUE;
}

EFI_STATUS
EFIAPI
EnableLink(VOID)
{
  
  UINTN  ParfBase   = 0;
  UINTN  DmCoreBase = 0;

  if (initializePCIe0) EnableLinkWithAddr(0xfc520000, 0xff000000, &isPCIe0LinkUp);
  if (initializePCIe1) EnableLinkWithAddr(0xfc528000, 0xf8800000, &isPCIe1LinkUp);
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "PCIe0 link: %d, PCIe1 link: %d\n", isPCIe0LinkUp, isPCIe1LinkUp));

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
  if (initializePCIe0 && isPCIe0LinkUp) ConfigureDmCoreWithBase(0xff000000);
  if (initializePCIe1 && isPCIe1LinkUp) ConfigureDmCoreWithBase(0xf8800000);

  // GPIO 36, in, function 2, 2mA drive, no pull (clkreq)
  mTlmmProtocol->SetFunction(36, 2);
  mTlmmProtocol->SetDriveStrength(36, 2);
  mTlmmProtocol->SetPull(36, GPIO_PULL_UP);
  mTlmmProtocol->DirectionInput(36);

  // GPIO 54, in, function 2, 2mA drive, no pull (clkreq)
  if (mBoardId == APQ8094)
  {
    mTlmmProtocol->SetFunction(54, 2);
    mTlmmProtocol->SetDriveStrength(54, 2);
    mTlmmProtocol->SetPull(54, GPIO_PULL_UP);
    mTlmmProtocol->DirectionInput(54);
  }

  return EFI_SUCCESS;
}

// WP-only configuration steps. Need more investigation
static inline void ConfigSpaceWithBase(UINTN DmCoreBase, UINTN Type, UINTN DeviceId)
{
  // UINT32 i          = 1;
  // UINT32 j          = 4;
  UINT32 k          = 0;
  UINT32 Addr       = 0;
  UINT32 Val        = 0;

  // DM_CORE, base varies
  // PCIE20_PLR_IATU_VIEWPORT (region index)
  MmioWrite32(DmCoreBase + 0x900, DeviceId);
  // PCIE20_PLR_IATU_CTRL1 (ATU_REG_TYPE difference?)
  MmioWrite32(DmCoreBase + 0x904, Type);
  // PCIE20_PLR_IATU_CTRL2
  MmioWrite32(DmCoreBase + 0x908, 0x80000000);
  // PCIE20_PLR_IATU_VIEWPORT
  Addr = DmCoreBase + 0x900 + /* (k << 20) */ + 0x100000;
  DEBUG((EFI_D_ERROR | EFI_D_INFO, "PCIE20_PLR_IATU_VIEWPORT: 0x%x\n", Addr));
  // PCIE20_PLR_IATU_LBAR (iATU Region Lower Base Address Register)
  MmioWrite32(DmCoreBase + 0x90c, Addr);
  // PCIE20_PLR_IATU_UBAR (iATU Region Upper Base Address Register)
  MmioWrite32(DmCoreBase + 0x910, 0);
  // PCIE20_PLR_IATU_LAR (iATU Region Limited Address Register)
  MmioWrite32(DmCoreBase + 0x914, Addr);
  // PCIE20_PLR_IATU_LTAR (iATU Region Lower Target Address Register)
  MmioWrite32(DmCoreBase + 0x918, /* ((k + 1) << 24) */ /* 0x01000000 */ 0x01000000);
  // PCIE20_PLR_IATU_UTAR (iATU Region Upper Target Address Register)
  MmioWrite32(DmCoreBase + 0x91c, 0);
  // PCIE20_BUS_NUM_REG
  // MmioWrite32(DmCoreBase + 0x18, 0x30100);
  // What's this?
  Val = MmioRead32(Addr + 0x188);
  MmioWrite32(Addr + 0x188, Val | 0xF);
  // PCIE20_DEVICE_CONTROL2_STATUS2
  Val = MmioRead32(Addr + 0x98);
  MmioWrite32(Addr + 0x98, Val | 0x400);
  // Three bars
  // i++;
  // if (k) j = 5;
}

EFI_STATUS
EFIAPI
ConfigSpace(VOID)
{
  if (initializePCIe0 && isPCIe0LinkUp) ConfigSpaceWithBase(0xff000000, PCIE20_CTRL1_TYPE_CFG0, 0);
  if (initializePCIe1 && isPCIe1LinkUp) ConfigSpaceWithBase(0xf8800000, PCIE20_CTRL1_TYPE_CFG0, 1);

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
  if (initializePCIe0 && isPCIe0LinkUp) ConfigBARWithBase(0xff000000);
  // if (initializePCIe1 && isPCIe1LinkUp) ConfigBARWithBase(0xf8800000);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PCIExpressConfiguratorEntry(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry ->\n"));

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> AcquireEfiProtocols\n"));
  Status = AcquireEfiProtocols();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- AcquireEfiProtocols\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> VerifyPlatform\n"));
  Status = VerifyPlatform();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- VerifyPlatform\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> EnableClocksMsm8994\n"));
  Status = EnableClocksMsm8994();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- EnableClocksMsm8994\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> ConfigurePCIeAndCnssGpio\n"));
  Status = ConfigurePCIeAndCnssGpio();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- ConfigurePCIeAndCnssGpio\n"));
  if (EFI_ERROR(Status))
    goto exit;

  // TODO: Call msm_pcie_restore_sec_config to restore security config
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> InitializePciePHY\n"));
  Status = InitializePciePHY();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- InitializePciePHY\n"));
  if (EFI_ERROR(Status))
    goto exit;

#ifdef ENABLE_QCOM_RPM
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> RpmTurnOnLdo30\n"));
  Status = RpmTurnOnLdo30();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- RpmTurnOnLdo30\n"));
  if (EFI_ERROR(Status))
    goto exit;
#endif

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> SetPipeClock\n"));
  Status = SetPipeClock();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- SetPipeClock\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> WaitForPCIePHYReady\n"));
  Status = WaitForPCIePHYReady();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- WaitForPCIePHYReady\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> RestoreSecurityConfig\n"));
  Status = RestoreSecurityConfig();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- RestoreSecurityConfig\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> EnableLink\n"));
  Status = EnableLink();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- EnableLink\n"));
  if (EFI_ERROR(Status))
    goto exit;

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> ConfigDmCore\n"));
  Status = ConfigDmCore();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- ConfigDmCore\n"));
  if (EFI_ERROR(Status))
    goto exit;

  if (mBoardId != APQ8094)
  {
    DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> ConfigSpace\n"));
    Status = ConfigSpace();
    DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- ConfigSpace\n"));
    if (EFI_ERROR(Status))
      goto exit;

    DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> FinishingUp\n"));
    Status = FinishingUp();
    DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- FinishingUp\n"));
    if (EFI_ERROR(Status))
      goto exit;
  }
  else
  {
    DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry -> ConfigSpace\n"));
    Status = ConfigSpace();
    DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <- ConfigSpace\n"));
    if (EFI_ERROR(Status))
      goto exit;
  }
  
  Status = gBS->InstallProtocolInterface(
      &ImageHandle, &gQcomMsmPCIExpressInitProtocolGuid, EFI_NATIVE_INTERFACE,
      &ImageHandle);

exit:
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "PCIExpressConfiguratorEntry <-\n"));
  return Status;
}
