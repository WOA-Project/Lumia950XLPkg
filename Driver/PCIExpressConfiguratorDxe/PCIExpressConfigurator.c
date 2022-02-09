#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/ArmLib.h>
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

QCOM_GPIO_TLMM_PROTOCOL *mTlmmProtocol;
#ifdef ENABLE_QCOM_RPM
QCOM_RPM_PROTOCOL *mRpmProtocol;
#endif
QCOM_PM8X41_PROTOCOL *mPmicProtocol;
QCOM_BOARD_PROTOCOL * mBoardProtocol;

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

exit:
  return Status;
}

EFI_STATUS
EFIAPI
VerifyPlatform(VOID)
{
  if (mBoardProtocol->board_platform_id() != MSM8994 &&
      mBoardProtocol->board_platform_id() != MSM8992) {
    DEBUG(
        (EFI_D_ERROR | EFI_D_WARN,
         "Target platform is not MSM8992 or MSM8994. PCIe init skipped \n"));
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
EnableClocksMsm8994(VOID)
{
  EFI_STATUS Status = EFI_SUCCESS;

  if (mBoardProtocol->board_platform_id() == MSM8992) {
    // HWIO_GCC_PCIE_0_PIPE_CBCR_ADDR
    MmioWrite32(0xFC401B14, 0);
    // HWIO_GCC_PCIE_0_PIPE_CMD_RCGR_ADDR
    MmioWrite32(0xFC401B18, 0);
  }
  else {
    // HWIO_GCC_PCIE_1_PIPE_CBCR_ADDR
    MmioWrite32(0xFC401B94, 0);
    // HWIO_GCC_PCIE_1_PIPE_CMD_RCGR_ADDR
    MmioWrite32(0xFC401B98, 0);
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
  if (mBoardProtocol->board_platform_id() == MSM8994)
    gdsc_pcie1_enable();
    // pcie_1_ref_clk_src
#ifdef ENABLE_QCOM_RPM
  rpm_smd_ln_bb_clk_enable();
#endif
  // pcie_1_aux_clk
  pcie_0_aux_clk_set_rate_and_enable();
  if (mBoardProtocol->board_platform_id() == MSM8994)
    pcie_1_aux_clk_set_rate_and_enable();
  // pcie_1_cfg_ahb_clk
  pcie_0_cfg_ahb_clk_enable();
  if (mBoardProtocol->board_platform_id() == MSM8994)
    pcie_1_cfg_ahb_clk_enable();
  // pcie_1_mstr_axi_clk
  pcie_0_mstr_axi_clk_enable();
  if (mBoardProtocol->board_platform_id() == MSM8994)
    pcie_1_mstr_axi_clk_enable();
  // pcie_1_slv_axi_clk
  pcie_0_slv_axi_clk_enable();
  if (mBoardProtocol->board_platform_id() == MSM8994)
    pcie_1_slv_axi_clk_enable();
  // pcie_1_phy_ldo
  pcie_0_phy_ldo_enable();
  if (mBoardProtocol->board_platform_id() == MSM8994)
    pcie_1_phy_ldo_enable();
  // pcie_phy_1_reset
  pcie_phy_0_reset_enable();
  if (mBoardProtocol->board_platform_id() == MSM8994)
    pcie_phy_1_reset_enable();
  // Memory fence
  MemoryFence();

  // exit:
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

  // Assert GPIO 35 PERST#
  mTlmmProtocol->SetFunction(35, 0);
  mTlmmProtocol->SetDriveStrength(35, 2);
  mTlmmProtocol->SetPull(35, GPIO_PULL_NONE);
  mTlmmProtocol->DirectionOutput(35, 1);
  mTlmmProtocol->Set(35, 1);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InitializePciePHY(VOID)
{
  UINTN MsmPciePhyBase = 0;

  if (mBoardProtocol->board_platform_id() == MSM8992) {
    // GCC clks (GCC_PCIE_0_BCR)
    MmioWrite32(0xFC401AC0, 0x1);
    gBS->Stall(2000);
    MmioWrite32(0xFC401AC0, 0x0);
    // PHY base
    MsmPciePhyBase = 0xfc526000;
  }
  else {
    // GCC clks (GCC_PCIE_1_BCR)
    MmioWrite32(0xFC401B40, 0x1);
    gBS->Stall(2000);
    MmioWrite32(0xFC401B40, 0x0);
    // PHY base
    MsmPciePhyBase = 0xfc52e000;
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

  // Wait for PHY to get ready
  while (MmioRead32(MsmPciePhyBase + PCIE_PHY_PCS_STATUS) & 0x40)
    gBS->Stall(5);

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

EFI_STATUS
EFIAPI
EnableLink(VOID)
{
  UINT32 Val        = 0;
  UINTN  ParfBase   = 0;
  UINTN  DmCoreBase = 0;

  if (mBoardProtocol->board_platform_id() == MSM8992) {
    // PARF: 0xfc520000, DM Core: 0xff000000
    ParfBase   = 0xfc520000;
    DmCoreBase = 0xff000000;
  }
  else {
    // PARF: 0xfc528000, DM Core: 0xf8800000
    ParfBase   = 0xfc528000;
    DmCoreBase = 0xf8800000;
  }

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

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ConfigDmCore(VOID)
{
  UINT32 Val        = 0;
  UINTN  DmCoreBase = 0;

  if (mBoardProtocol->board_platform_id() == MSM8992) {
    // DM Core: 0xff000000
    DmCoreBase = 0xff000000;
  }
  else {
    // DM Core: 0xf8800000
    DmCoreBase = 0xf8800000;
  }

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

  // GPIO 36, in, function 2, 2mA drive, no pull (clkreq)
  mTlmmProtocol->SetFunction(36, 2);
  mTlmmProtocol->SetDriveStrength(36, 2);
  mTlmmProtocol->SetPull(36, GPIO_PULL_NONE);
  mTlmmProtocol->DirectionInput(36);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ConfigSpace(VOID)
{
  UINT32 i          = 1;
  UINT32 j          = 4;
  UINT32 k          = 0;
  UINT32 Addr       = 0;
  UINT32 Val        = 0;
  UINTN  DmCoreBase = 0;

  if (mBoardProtocol->board_platform_id() == MSM8992) {
    // DM Core: 0xff000000
    DmCoreBase = 0xff000000;
  }
  else {
    // DM Core: 0xf8800000
    DmCoreBase = 0xf8800000;
  }

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

  return EFI_SUCCESS;
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

  if (mBoardProtocol->board_platform_id() == MSM8992) {
    // DM Core: 0xff000000
    DmCoreBase = 0xff000000;
  }
  else {
    // DM Core: 0xf8800000
    DmCoreBase = 0xf8800000;
  }

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
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PCIExpressConfiguratorEntry(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  Status = AcquireEfiProtocols();
  if (EFI_ERROR(Status))
    goto exit;

  Status = VerifyPlatform();
  if (EFI_ERROR(Status))
    goto exit;

  Status = EnableClocksMsm8994();
  if (EFI_ERROR(Status))
    goto exit;

  Status = ConfigurePCIeAndCnssGpio();
  if (EFI_ERROR(Status))
    goto exit;

  // TODO: Call msm_pcie_restore_sec_config to restore security config

  Status = InitializePciePHY();
  if (EFI_ERROR(Status))
    goto exit;

#ifdef ENABLE_QCOM_RPM
  Status = RpmTurnOnLdo30();
  if (EFI_ERROR(Status))
    goto exit;
#endif

  Status = SetPipeClock();
  if (EFI_ERROR(Status))
    goto exit;

  Status = EnableLink();
  if (EFI_ERROR(Status))
    goto exit;

  Status = ConfigDmCore();
  if (EFI_ERROR(Status))
    goto exit;

  Status = ConfigSpace();
  if (EFI_ERROR(Status))
    goto exit;

  Status = FinishingUp();
  if (EFI_ERROR(Status))
    goto exit;

  Status = gBS->InstallProtocolInterface(
      &ImageHandle, &gQcomMsmPCIExpressInitProtocolGuid, EFI_NATIVE_INTERFACE,
      &ImageHandle);

exit:
  return Status;
}
