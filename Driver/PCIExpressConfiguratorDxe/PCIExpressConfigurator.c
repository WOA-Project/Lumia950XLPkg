#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <IndustryStandard/Pci22.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <GplLibrary/clock-gcc-msm8994.h>
#include <GplLibrary/clock-rpm-msm8994.h>
#include <GplLibrary/gdsc-msm8994.h>

#include <Chipset/PCIExpress.h>
#include <Platform/iomap.h>

#include <Protocol/QcomGpioTlmm.h>
#include <Protocol/QcomRpm.h>
#include <Protocol/QcomPm8x41.h>
#include <Protocol/QcomBoard.h>

#include "BitOps.h"
#include "PCIeDefinition.h"

QCOM_GPIO_TLMM_PROTOCOL *mTlmmProtocol;
QCOM_RPM_PROTOCOL *mRpmProtocol;
QCOM_PM8X41_PROTOCOL *mPmicProtocol;
QCOM_BOARD_PROTOCOL *mBoardProtocol;

EFI_STATUS
EFIAPI
AcquireEfiProtocols(
	VOID
)
{
	EFI_STATUS Status;
	Status = gBS->LocateProtocol(
		&gQcomGpioTlmmProtocolGuid,
		NULL,
		(VOID **) &mTlmmProtocol
	);
	if (EFI_ERROR(Status)) goto exit;

	Status = gBS->LocateProtocol(
		&gQcomRpmProtocolGuid,
		NULL,
		(VOID **) &mRpmProtocol
	);
	if (EFI_ERROR(Status)) goto exit;

	Status = gBS->LocateProtocol(
		&gQcomPm8x41ProtocolGuid,
		NULL,
		(VOID**) &mPmicProtocol
	);
	if (EFI_ERROR(Status)) goto exit;

	Status = gBS->LocateProtocol(
		&gQcomBoardProtocolGuid,
		NULL,
		(VOID**) &mBoardProtocol
	);
	if (EFI_ERROR(Status)) goto exit;

exit:
	return Status;
}

EFI_STATUS
EFIAPI
VerifyPlatform(
	VOID
)
{
	if (mBoardProtocol->board_platform_id() != MSM8994)
	{
		DEBUG((EFI_D_ERROR | EFI_D_WARN, "Target platform is not MSM8994. PCIe init skipped \n"));
		return EFI_UNSUPPORTED;
	}

	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
EnableClocksMsm8994(
	VOID
)
{
	EFI_STATUS Status;

	// HWIO_GCC_PCIE_1_PIPE_CBCR_ADDR
	MmioWrite32(0xFC401B94, 0);
	// HWIO_GCC_PCIE_1_PIPE_CMD_RCGR_ADDR
	MmioWrite32(0xFC401B98, 0);

	// Clocks & LDOs
	// Power and Clock
	// GDSC & regulator
	// LDO12 (1.8V), LDO28 (0.9V)
	Status = mRpmProtocol->rpm_ldo_pipe_enable();
	if (EFI_ERROR(Status)) goto exit;
	gBS->Stall(1000);

	// GDSC
	gdsc_pcie0_enable();
	gdsc_pcie1_enable();
	// pcie_1_ref_clk_src
	rpm_smd_ln_bb_clk_enable();
	// pcie_1_aux_clk
	pcie_0_aux_clk_set_rate_and_enable();
	pcie_1_aux_clk_set_rate_and_enable();
	// pcie_1_cfg_ahb_clk
	pcie_0_cfg_ahb_clk_enable();
	pcie_1_cfg_ahb_clk_enable();
	// pcie_1_mstr_axi_clk
	pcie_0_mstr_axi_clk_enable();
	pcie_1_mstr_axi_clk_enable();
	// pcie_1_slv_axi_clk
	pcie_0_slv_axi_clk_enable();
	pcie_1_slv_axi_clk_enable();
	// pcie_1_phy_ldo
	pcie_0_phy_ldo_enable();
	pcie_1_phy_ldo_enable();
	// pcie_phy_1_reset
	pcie_phy_0_reset_enable();
	pcie_phy_1_reset_enable();
	// Memory fence
	MemoryFence();

exit:
	return Status;
}

EFI_STATUS
EFIAPI
ConfigurePCIeAndCnssGpio(
	VOID
)
{
	// GPIO 35 PERST#, func Generic I/O (0), Dir Out, No Pull (0), Drive 2mA (0), assert -> deassert
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

	// GPIO 112 for CNSS bootstrap: generic I/O, dir out, pull up, drive 2mA, keep assert
	mTlmmProtocol->SetFunction(112, 0);
	mTlmmProtocol->SetDriveStrength(112, 2);
	mTlmmProtocol->SetPull(112, GPIO_PULL_UP);
	mTlmmProtocol->DirectionOutput(112, 1);
	mTlmmProtocol->Set(112, 1);

	// WLAN_LDO_3V_CTRL (PM8994 GPIO9) configure as out
	// Skip?

	// GPIO_19 BT (PM8994 GPIO19) configure as out
	// Skip?

	// GPIO 113 for WLAN_EN: generic I/O, dir out, no pull, drive 2mA, assert -> deassert
	mTlmmProtocol->SetFunction(113, 0);
	mTlmmProtocol->SetDriveStrength(113, 2);
	mTlmmProtocol->SetPull(113, GPIO_PULL_NONE);
	mTlmmProtocol->DirectionOutput(113, 1);
	mTlmmProtocol->Set(113, 0);

	// GPIO 50 Unknown: generic I/O, dir out, no pull, drive 2mA, assert -> deassert
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
InitializePciePHY(
	VOID
)
{
	// GCC clks
	MmioWrite32(0xFC401B40, 0x1);
	gBS->Stall(2000);
	MmioWrite32(0xFC401B40, 0x0);

	// Program PHY
	// PCIE_PHY_POWER_DOWN_CONTROL
	MmioWrite32(0xFC52E604, 0x3);
	// QSERDES_COM_SYSCLK_EN_SEL_TXBAND
	MmioWrite32(0xFC52E048, 0x8);
	// QSERDES_COM_DEC_START1
	MmioWrite32(0xFC52E0AC, 0x82);
	// QSERDES_COM_DEC_START2
	MmioWrite32(0xFC52E10C, 0x3);
	// QSERDES_COM_DIV_FRAC_START1
	MmioWrite32(0xFC52E100, 0x0D5);
	// QSERDES_COM_DIV_FRAC_START2
	MmioWrite32(0xFC52E104, 0x0AA);
	// QSERDES_COM_DIV_FRAC_START3
	MmioWrite32(0xFC52E108, 0x4D);
	MmioWrite32(0xFC52E09C, 0x7);
	MmioWrite32(0xFC52E090, 0x41);
	MmioWrite32(0xFC52E094, 0x3);
	MmioWrite32(0xFC52E114, 0x7C);
	MmioWrite32(0xFC52E034, 0x7);
	MmioWrite32(0xFC52E038, 0x1F);
	MmioWrite32(0xFC52E03C, 0xF);
	MmioWrite32(0xFC52E024, 0x1);
	MmioWrite32(0xFC52E00C, 0xF);
	MmioWrite32(0xFC52E010, 0xF);
	MmioWrite32(0xFC52E014, 0x46);
	MmioWrite32(0xFC52E400, 0xF5);
	MmioWrite32(0xFC52E408, 0x2C);
	MmioWrite32(0xFC52E04C, 0x91);
	MmioWrite32(0xFC52E050, 0x7);
	MmioWrite32(0xFC52E004, 0x0E1);
	MmioWrite32(0xFC52E0E0, 0x24);
	MmioWrite32(0xFC52E0E8, 0x77);
	MmioWrite32(0xFC52E0F0, 0x15);
	MmioWrite32(0xFC52E268, 0x3);
	MmioWrite32(0xFC52E4A8, 0xFF);
	MmioWrite32(0xFC52E4AC, 0x7);
	MmioWrite32(0xFC52E4B0, 0xFF);
	MmioWrite32(0xFC52E4B4, 0x0);
	MmioWrite32(0xFC52E4BC, 0x1E);
	MmioWrite32(0xFC52E4F0, 0x67);
	MmioWrite32(0xFC52E4F4, 0x80);
	MmioWrite32(0xFC52E4F8, 0x40);
	MmioWrite32(0xFC52E500, 0x70);
	MmioWrite32(0xFC52E504, 0x0C);
	MmioWrite32(0xFC52E0B4, 0x1);
	MmioWrite32(0xFC52E0B8, 0x2);
	MmioWrite32(0xFC52E0C0, 0x31);
	MmioWrite32(0xFC52E0C4, 0x1);
	MmioWrite32(0xFC52E0C8, 0x19);
	MmioWrite32(0xFC52E0CC, 0x19);
	MmioWrite32(0xFC52E110, 0x10);
	MmioWrite32(0xFC52E648, 0x10);
	MmioWrite32(0xFC52E650, 0x23);
	// PCIE_PHY_POWER_STATE_CONFIG2
	MmioWrite32(0xFC52E654, 0x4B);
	// PCIE_PHY_RX_IDLE_DTCT_CNTRL
	MmioWrite32(0xFC52E64C, 0x4D);
	// PCIE_PHY_SW_RESET
	MmioWrite32(0xFC52E600, 0x0);
	// PCIE_PHY_START
	MmioWrite32(0xFC52E608, 0x3);

	// Wait for PHY to get ready
	while (MmioRead32(0xFC52E728) & 0x40) 
	{
		gBS->Stall(5);
	}

	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RpmTurnOnLdo30(
	VOID
)
{
	return mRpmProtocol->rpm_ldo30_enable();
}

EFI_STATUS
EFIAPI
SetPipeClock(
	VOID
)
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
EnableLink(
	VOID
)
{
	UINT32 Val = 0;

	MmioWrite32(0xFC529000, 0x4);
	MmioWrite32(0xF8800F24, 0x1);
	MmioWrite32(0xF8800088, 0x1000000);
	gBS->Stall(1000);

	Val = MmioRead32(0xF8800004);
	MmioWrite32(0xF8800004, Val | 0x46);

	Val = MmioRead32(0xF88000A0);
	MmioWrite32(0xF88000A0, (Val & 0xFFFFFFF0) + 1);
	MmioWrite32(0xFC5281B0, 0x100);

	// Check link (PCIE20_ELBI_SYS_STTS + 0x8)
	while ((MmioRead32(0xF8800F28) & 0x400) != 0x400) gBS->Stall(1000);
	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ConfigDmCore(
	VOID
)
{
	UINT32 Val = 0;

	MmioWrite32(0xF88008BC, 0x1);
	Val = MmioRead32(0xF880007C);
	MmioWrite32(0xF880007C, (Val & 0xFFFFFBFF) | 0x400800);
	MmioWrite32(0xF88008BC, 0x0);
	Val = MmioRead32(0xF8800154);
	MmioWrite32(0xF8800154, Val | 0xF);
	Val = MmioRead32(0xF8800158);
	MmioWrite32(0xF8800158, Val | 0xF);
	Val = MmioRead32(0xF8800098);
	MmioWrite32(0xF8800098, 0x400);

	// GPIO 36, in, function 2, 2mA drive, no pull (clkreq)
	mTlmmProtocol->SetFunction(36, 2);
	mTlmmProtocol->SetDriveStrength(36, 2);
	mTlmmProtocol->SetPull(36, GPIO_PULL_NONE);
	mTlmmProtocol->DirectionInput(36);

	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ConfigSpace(
	VOID
)
{
	UINT32 i = 1;
	UINT32 j = 4;
	UINT32 k = 0;
	UINT32 Addr = 0;
	UINT32 Val = 0;

	while (TRUE)
	{
		// Base: 0xF8800000, DM_CORE
		MmioWrite32(0xF8800900, i);
		MmioWrite32(0xF8800904, j);
		MmioWrite32(0xF8800908, 0x80000000);
		Addr = 0xF8800900 + (k << 20) + 0x100000;
		MmioWrite32(0xF880090C, Addr);
		MmioWrite32(0xF8800910, 0);
		MmioWrite32(0xF8800914, Addr);
		MmioWrite32(0xF8800918, ((k + 1) << 24));
		MmioWrite32(0xF880091C, 0);
		MmioWrite32(0xF8800018, 0x30100);
		Val = MmioRead32(Addr + 0x188);
		MmioWrite32(Addr + 0x188, Val | 0xF);
		Val = MmioRead32(Addr + 0x98);
		MmioWrite32(Addr + 0x98, Val | 0x400);
		// Three bars
		++i;
		if (++k < 2) break;
		if (k) j = 5;
	}

	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FinishingUp(
	VOID
)
{
	UINT32 i = 0;
	UINT32 j = 0;
	UINT32 Addr = 0xF8800000 + 0x100000;
	UINT32 Adr1 = 0xF8800000 + 0x400000;
	UINT32 k = 16;
	UINT32 Val = 0;
	BOOLEAN SetI = FALSE;

	do
	{
		MmioWrite32(Addr + k, 0xFFFFFFFF);
		Val = MmioRead32(Addr + k);
		if (Val && !(Val << 31))
		{
			if (Val & 8)
			{
				SetI = TRUE;
			}
			else
			{
				SetI = FALSE;
			}

			if (((Val >> 1) & 3) != 2 || (k += 4, MmioWrite32(Addr + k, 0xFFFFFFFF), MmioRead32(Addr + k) == -1))
			{
				if (SetI)
				{
					i = i - (Val & 0xFFFFFFF0);
				}
				else
				{
					j = j - (Val & 0xFFFFFFF0);
				}
			}
		}
		k += 4;
	} while (k <= 0x24);

	MmioWrite32(0xF8800020, (((Adr1 + j) >> 16) << 16) | (Adr1 >> 16));
	MmioWrite32(0xF8800024, ((Adr1 + j + i) & 0xFFFF0000) | ((Adr1 + j + i) >> 16));
	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PCIExpressConfiguratorEntry(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS Status;
	
	Status = AcquireEfiProtocols();
	if (EFI_ERROR(Status)) goto exit;

	Status = VerifyPlatform();
	if (EFI_ERROR(Status)) goto exit;

	Status = EnableClocksMsm8994();
	if (EFI_ERROR(Status)) goto exit;

	Status = ConfigurePCIeAndCnssGpio();
	if (EFI_ERROR(Status)) goto exit;

	Status = InitializePciePHY();
	if (EFI_ERROR(Status)) goto exit;

	Status = RpmTurnOnLdo30();
	if (EFI_ERROR(Status)) goto exit;

	Status = SetPipeClock();
	if (EFI_ERROR(Status)) goto exit;

	Status = EnableLink();
	if (EFI_ERROR(Status)) goto exit;

	Status = ConfigDmCore();
	if (EFI_ERROR(Status)) goto exit;

	Status = ConfigSpace();
	if (EFI_ERROR(Status)) goto exit;

	Status = FinishingUp();
	if (EFI_ERROR(Status)) goto exit;

	Status = gBS->InstallProtocolInterface(
		&ImageHandle,
		&gQcomMsmPCIExpressInitProtocolGuid,
		EFI_NATIVE_INTERFACE,
		&ImageHandle
	);

exit:
	return Status;
}
