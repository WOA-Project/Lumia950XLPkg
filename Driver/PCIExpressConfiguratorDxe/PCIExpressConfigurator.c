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
#include <Protocol/HardwareInterrupt.h>

#include "BitOps.h"
#include "TrustZone.h"
#include "TzSyscall.h"

QCOM_GPIO_TLMM_PROTOCOL *mTlmmProtocol;
QCOM_RPM_PROTOCOL *mRpmProtocol;
EFI_HARDWARE_INTERRUPT_PROTOCOL *gInterrupt = NULL;

STATIC UINT8 mCurrentBDF;

static inline void WriteRegisterField(
	void *base,
	uint32_t offset,
	const uint32_t mask,
	uint32_t val
)
{
	uint32_t shift = find_first_bit((void *)&mask, 32);
	uint32_t tmp = readl_relaxed(base + offset);

	tmp &= ~mask; /* clear written bits */
	val = tmp | (val << shift);
	writel_relaxed(val, base + offset);
	MemoryFence();
}

static inline void WriteRegister(
	void *base, 
	UINT32 offset, 
	UINT32 value
)
{
	writel_relaxed(value, base + offset);
	MemoryFence();
}

static void WriteMask(
	void *addr,
	uint32_t clear_mask,
	uint32_t set_mask
)
{
	uint32_t val;
	val = (readl_relaxed(addr) & ~clear_mask) | set_mask;
	writel_relaxed(val, addr);

	/* ensure data is written to hardware register */
	ArmDataMemoryBarrier();
}

static void PhyInit(
	VOID
)
{
	DEBUG((EFI_D_ERROR, "RC1: Initializing 20nm QMP phy - 19.2MHz \n"));

	void* PhyAddress = (void*) MSM_PCIE_RES_PHY;

	WriteRegister(PhyAddress, PCIE_PHY_POWER_DOWN_CONTROL, 0x03);

	WriteRegister(PhyAddress, QSERDES_COM_SYSCLK_EN_SEL_TXBAND, 0x08);
	WriteRegister(PhyAddress, QSERDES_COM_DEC_START1, 0x82);
	WriteRegister(PhyAddress, QSERDES_COM_DEC_START2, 0x03);
	WriteRegister(PhyAddress, QSERDES_COM_DIV_FRAC_START1, 0xD5);
	WriteRegister(PhyAddress, QSERDES_COM_DIV_FRAC_START2, 0xAA);
	WriteRegister(PhyAddress, QSERDES_COM_DIV_FRAC_START3, 0x4D);
	WriteRegister(PhyAddress, QSERDES_COM_PLLLOCK_CMP_EN, 0x03);
	WriteRegister(PhyAddress, QSERDES_COM_PLLLOCK_CMP1, 0x06);
	WriteRegister(PhyAddress, QSERDES_COM_PLLLOCK_CMP2, 0x1A);
	WriteRegister(PhyAddress, QSERDES_COM_PLL_CRCTRL, 0x7C);
	WriteRegister(PhyAddress, QSERDES_COM_PLL_CP_SETI, 0x1F);
	WriteRegister(PhyAddress, QSERDES_COM_PLL_IP_SETP, 0x12);
	WriteRegister(PhyAddress, QSERDES_COM_PLL_CP_SETP, 0x0F);
	WriteRegister(PhyAddress, QSERDES_COM_PLL_IP_SETI, 0x01);

	WriteRegister(PhyAddress, QSERDES_COM_IE_TRIM, 0x0F);
	WriteRegister(PhyAddress, QSERDES_COM_IP_TRIM, 0x0F);

	WriteRegister(PhyAddress, QSERDES_COM_PLL_CNTRL, 0x46);

	/* CDR Settings */
	WriteRegister(PhyAddress, QSERDES_RX_CDR_CONTROL1, 0xF4);
	WriteRegister(PhyAddress, QSERDES_RX_CDR_CONTROL_HALF, 0x2C);

	WriteRegister(PhyAddress, QSERDES_COM_PLL_VCOTAIL_EN, 0xE1);

	/* Calibration Settings */
	WriteRegister(PhyAddress, QSERDES_COM_RESETSM_CNTRL, 0x91);
	WriteRegister(PhyAddress, QSERDES_COM_RESETSM_CNTRL2, 0x07);

	/* Additional writes */
	WriteRegister(PhyAddress, QSERDES_COM_RES_CODE_START_SEG1, 0x20);
	WriteRegister(PhyAddress, QSERDES_COM_RES_CODE_CAL_CSR, 0x77);
	WriteRegister(PhyAddress, QSERDES_COM_RES_TRIM_CONTROL, 0x15);
	WriteRegister(PhyAddress, QSERDES_TX_RCV_DETECT_LVL, 0x03);
	WriteRegister(PhyAddress, QSERDES_RX_UCDR_FO_GAIN, 0x09);
	WriteRegister(PhyAddress, QSERDES_RX_UCDR_SO_GAIN, 0x04);
	WriteRegister(PhyAddress, QSERDES_RX_UCDR_SO_SATURATION_AND_ENABLE,
		0x49);
	WriteRegister(PhyAddress, QSERDES_RX_RX_EQ_GAIN1_LSB, 0xFF);
	WriteRegister(PhyAddress, QSERDES_RX_RX_EQ_GAIN1_MSB, 0x1F);
	WriteRegister(PhyAddress, QSERDES_RX_RX_EQ_GAIN2_LSB, 0xFF);
	WriteRegister(PhyAddress, QSERDES_RX_RX_EQ_GAIN2_MSB, 0x00);
	WriteRegister(PhyAddress, QSERDES_RX_RX_EQU_ADAPTOR_CNTRL2, 0x1E);
	WriteRegister(PhyAddress, QSERDES_RX_RX_EQ_OFFSET_ADAPTOR_CNTRL1,
		0x67);
	WriteRegister(PhyAddress, QSERDES_RX_RX_OFFSET_ADAPTOR_CNTRL2, 0x80);
	WriteRegister(PhyAddress, QSERDES_RX_SIGDET_ENABLES, 0x40);
	WriteRegister(PhyAddress, QSERDES_RX_SIGDET_CNTRL, 0xB0);
	WriteRegister(PhyAddress, QSERDES_RX_SIGDET_DEGLITCH_CNTRL, 0x06);
	WriteRegister(PhyAddress, QSERDES_COM_PLL_RXTXEPCLK_EN, 0x10);
	WriteRegister(PhyAddress, PCIE_PHY_ENDPOINT_REFCLK_DRIVE, 0x10);
	WriteRegister(PhyAddress, PCIE_PHY_POWER_STATE_CONFIG1, 0xA3);
	WriteRegister(PhyAddress, PCIE_PHY_POWER_STATE_CONFIG2, 0x4B);
	WriteRegister(PhyAddress, PCIE_PHY_RX_IDLE_DTCT_CNTRL, 0x4D);

	WriteRegister(PhyAddress, PCIE_PHY_SW_RESET, 0x00);
	WriteRegister(PhyAddress, PCIE_PHY_START, 0x03);
}

static bool pcie_phy_is_ready()
{
	if (readl_relaxed(MSM_PCIE_RES_PHY + PCIE_PHY_PCS_STATUS) & BIT(6))
		return false;
	else
		return true;
}

STATIC
BOOLEAN
ConfirmLinkUp(
	BOOLEAN check_sw_stts,
	BOOLEAN check_ep
)
{
	uint32_t val;

	if (!(readl_relaxed(MSM_PCIE_RES_DM_CORE + 0x80) & BIT(29)))
	{
		DEBUG((EFI_D_ERROR, "PCIe: The link of RC1 is not up.\n"));
		return false;
	}

	val = readl_relaxed(MSM_PCIE_RES_DM_CORE);
	DEBUG((EFI_D_ERROR, "PCIe: device ID and vender ID of RC1 are %x:%x \n", (UINT16) (val >> 16), (UINT16) val));

	if (val == PCIE_LINK_DOWN)
	{
		DEBUG((EFI_D_ERROR, "PCIe: The link of RC1 is not really up; device ID and vender ID of RC1 are 0x%x.\n", val));
		return false;
	}

	return true;
}

STATIC
EFI_STATUS
EFIAPI
WaitForLink(
	VOID
)
{
	uint32_t link_check_count = 0;
	uint32_t val = 0;

	do {
		DEBUG((EFI_D_ERROR, "Waiting for PCIe link to come up... %d \n", link_check_count));
		gBS->Stall(LINK_UP_TIMEOUT_US_MAX);
		val = readl_relaxed((void*) MSM_PCIE_RES_ELBI + PCIE20_ELBI_SYS_STTS);
	} while ((!(val & XMLH_LINK_UP) || !ConfirmLinkUp(false, false))
		&& (link_check_count++ < LINK_UP_CHECK_MAX_COUNT));

	if ((val & XMLH_LINK_UP) && ConfirmLinkUp(false, false))
	{
		DEBUG((EFI_D_ERROR, "Link is up after %d checkings \n", link_check_count));
		DEBUG((EFI_D_ERROR, "PCIe RC1 link initialized \n"));
		return EFI_SUCCESS;
	}
	else
	{
		DEBUG((EFI_D_ERROR, "PCIe RC1 link initialization failed\n"));
		ASSERT(FALSE);
	}

	return EFI_NOT_READY;
}

static void ConfigIATU(
	int nr,
	uint8_t type,
	unsigned long host_addr,
	uint32_t host_end,
	unsigned long target_addr
)
{
	void *pcie20 = (void*) MSM_PCIE_RES_DM_CORE;

	/* select region */
	writel_relaxed(nr, pcie20 + PCIE20_PLR_IATU_VIEWPORT);
	/* ensure that hardware locks it */
	ArmDataMemoryBarrier();

	/* switch off region before changing it */
	writel_relaxed(0, pcie20 + PCIE20_PLR_IATU_CTRL2);
	/* and wait till it propagates to the hardware */
	ArmDataMemoryBarrier();

	writel_relaxed(type, pcie20 + PCIE20_PLR_IATU_CTRL1);
	writel_relaxed(lower_32_bits(host_addr), pcie20 + PCIE20_PLR_IATU_LBAR);
	writel_relaxed(upper_32_bits(host_addr), pcie20 + PCIE20_PLR_IATU_UBAR);
	writel_relaxed(host_end, pcie20 + PCIE20_PLR_IATU_LAR);
	writel_relaxed(lower_32_bits(target_addr), pcie20 + PCIE20_PLR_IATU_LTAR);
	writel_relaxed(upper_32_bits(target_addr), pcie20 + PCIE20_PLR_IATU_UTAR);
	ArmDataMemoryBarrier();
	writel_relaxed(BIT(31), pcie20 + PCIE20_PLR_IATU_CTRL2);

	/* ensure that changes propagated to the hardware */
	ArmDataMemoryBarrier();
}

static void ConfigureBDF(
	uint8_t bus,
	uint8_t devfn
)
{
	uint32_t bdf = BDF_OFFSET(bus, devfn);
	uint8_t type = bus == 1 ? PCIE20_CTRL1_TYPE_CFG0 : PCIE20_CTRL1_TYPE_CFG1;
	if (mCurrentBDF == bdf) return;

	ConfigIATU(
		0, type,
		MSM_PCIE_RES_CONF,
		MSM_PCIE_RES_CONF + SZ_4K - 1,
		bdf
	);

	mCurrentBDF = bdf;
}

STATIC
VOID
ConfigureController(
	VOID
)
{
	/*
	* program and enable address translation region 0 (device config
	* address space); region type config;
	* axi config address range to device config address range
	*/
	ConfigureBDF(1, 0);

	/* Configure N_FTS */
	WriteMask((void*)MSM_PCIE_RES_DM_CORE + PCIE20_ACK_F_ASPM_CTRL_REG, 0, BIT(15));

	/* Enable AER on RC */
	WriteMask((void*)MSM_PCIE_RES_DM_CORE + PCIE20_BRIDGE_CTRL, 0, BIT(16) | BIT(17));
	WriteMask((void*)MSM_PCIE_RES_DM_CORE + PCIE20_CAP_DEVCTRLSTATUS, 0,
		BIT(3) | BIT(2) | BIT(1) | BIT(0)
	);

	DEBUG((EFI_D_ERROR, "RC1 PCIE20_CAP_DEVCTRLSTATUS:0x%x\n", readl_relaxed(MSM_PCIE_RES_DM_CORE + PCIE20_CAP_DEVCTRLSTATUS)));
}

static inline int CheckAlignment(
	uint32_t offset
)
{
	if (offset % 4)
	{
		DEBUG((EFI_D_ERROR, "PCIe: RC1: offset 0x%x is not correctly aligned\n", offset));
		return MSM_PCIE_ERROR;
	}

	return 0;
}

STATIC
VOID
ConfigureLinkState(
	VOID
)
{
	uint32_t val;
	uint32_t current_offset;
	uint32_t ep_link_cap_offset = 0;
	uint32_t ep_link_ctrlstts_offset = 0;
	uint32_t ep_dev_ctrl2stts2_offset = 0;

	/* Ignore: Enable the AUX Clock and the Core Clk to be synchronous for L1SS */

	current_offset = readl_relaxed(MSM_PCIE_RES_CONF + PCIE_CAP_PTR_OFFSET) & 0xff;
	while (current_offset)
	{
		if (CheckAlignment(current_offset)) return;

		val = readl_relaxed(MSM_PCIE_RES_CONF + current_offset);
		if ((val & 0xff) == PCIE20_CAP_ID)
		{
			ep_link_cap_offset = current_offset + 0x0c;
			ep_link_ctrlstts_offset = current_offset + 0x10;
			ep_dev_ctrl2stts2_offset = current_offset + 0x28;
			break;
		}
		current_offset = (val >> 8) & 0xff;
	}

	if (!ep_link_cap_offset)
	{
		DEBUG((EFI_D_ERROR, "RC1 endpoint does not support PCIe capability registers\n"));
		return;
	}
	else
	{
		DEBUG((EFI_D_ERROR, "RC1: ep_link_cap_offset: 0x%x\n", ep_link_cap_offset));
	}
}

EFI_STATUS
EFIAPI
PCIExpressConfiguratorEntry(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS Status;
	UINT32 Retries = 0;
	UINT64 Parameters[SCM_MAX_NUM_PARAMETERS] = { 0 };
	UINT32 InternalTrustedOsId = 0;
	UINT32 DirectParameters[NUM_DIRECT_REQUEST_PARAMETERS] = { 0 };
	SmcErrnoType TzStatus = SMC_SUCCESS;
	tz_restore_sec_cfg_req_t *pSysCallReq = (tz_restore_sec_cfg_req_t*) Parameters;

	/* Get TLMM Protocol */
	Status = gBS->LocateProtocol(
		&gQcomGpioTlmmProtocolGuid,
		NULL,
		(VOID **)&mTlmmProtocol
	);

	if (EFI_ERROR(Status))
	{
		DEBUG((EFI_D_ERROR, "QCOM TLMM protocol is not ready \n"));
		goto exit;
	}

	Status = gBS->LocateProtocol(
		&gQcomRpmProtocolGuid,
		NULL,
		(VOID **)&mRpmProtocol
	);

	if (EFI_ERROR(Status))
	{
		DEBUG((EFI_D_ERROR, "QCOM RPM protocol is not ready \n"));
		goto exit;
	}

	Status = gBS->LocateProtocol(
		&gHardwareInterruptProtocolGuid,
		NULL,
		(VOID **) &gInterrupt
	);

	if (EFI_ERROR(Status)) goto exit;

	// WLAN_EN?
	mTlmmProtocol->DirectionOutput(
		113,
		1
	);

	// Assert PCIe reset link to keep EP in reset
	mTlmmProtocol->Set(
		MSM_PCIE_GPIO_PERST_PIN,
		MSM_PCIE_GPIO_PERST_PIN_ON
	);

	gBS->Stall(
		PERST_PROPAGATION_DELAY_US_MAX
	);

	// Power and Clock
	// GDSC & regulator
	// LDO12 (1.8V), LDO28 (0.9V)
	Status = mRpmProtocol->rpm_ldo_pipe_enable();
	if (EFI_ERROR(Status)) goto exit;
	udelay(1000);

	// GDSC
	gdsc_pcie1_enable();

	// pcie_1_ref_clk_src
	rpm_smd_ln_bb_clk_enable();
	// pcie_1_aux_clk
	pcie_1_aux_clk_set_rate_and_enable();
	// pcie_1_cfg_ahb_clk
	pcie_1_cfg_ahb_clk_enable();
	// pcie_1_mstr_axi_clk
	pcie_1_mstr_axi_clk_enable();
	// pcie_1_slv_axi_clk
	pcie_1_slv_axi_clk_enable();
	// pcie_1_phy_ldo
	pcie_1_phy_ldo_enable();
	// pcie_phy_1_reset
	pcie_phy_1_reset_enable();
	// Memory fence
	MemoryFence();

	// Restore security config
	pSysCallReq->device = TZ_DEVICE_PCIE_1;
	pSysCallReq->spare = 0;

	DirectParameters[0] = Parameters[0];
	DirectParameters[1] = Parameters[1];

	TzStatus = tz_armv8_smc_call(
		TZ_RESTORE_SEC_CFG,
		TZ_RESTORE_SEC_CFG_PARAM_ID,
		DirectParameters,
		&InternalTrustedOsId
	);

	ASSERT(TzStatus == SMC_SUCCESS);

	// Enable PCIe clocks and resets
	WriteMask((void*)MSM_PCIE_RES_PARF + PCIE20_PARF_PHY_CTRL, BIT(0), 0);

	// 5. Change DBI base address
	writel_relaxed(0, MSM_PCIE_RES_PARF + PCIE20_PARF_DBI_BASE_ADDR);
	writel_relaxed(0x365E, MSM_PCIE_RES_PARF + PCIE20_PARF_SYS_CTRL);

	// 5.5 MSI is used
	WriteMask((void*)MSM_PCIE_RES_PARF + PCIE20_PARF_AXI_MSTR_WR_ADDR_HALT, 0, BIT(31));

	// 6. Initialize PHY
	PhyInit();

	// 6.5 Pipe clock
	gBS->Stall(REFCLK_STABILIZATION_DELAY_US_MAX);
	// pcie_1_pipe_clk
	pcie_1_pipe_clk_set_rate_enable();
	MemoryFence();

	// 7. Check
	DEBUG((EFI_D_ERROR, "RC1: waiting for phy ready...\n"));

	do {
		if (pcie_phy_is_ready()) break;
		Retries++;
		gBS->Stall(REFCLK_STABILIZATION_DELAY_US_MAX);
	} while (Retries < PHY_READY_TIMEOUT_COUNT);

	if (pcie_phy_is_ready())
	{
		DEBUG((EFI_D_ERROR, "PCIe: RC1 PHY is ready after %d tries! \n", Retries));
	}
	else
	{
		DEBUG((EFI_D_ERROR, " PCIe: RC1 PHY failed to come up! \n"));
		CpuDeadLoop();
	}

	// Wait for EP
	gBS->Stall(10000);

	// 8. De-assert PCIe reset link to bring EP out of reset
	DEBUG((EFI_D_ERROR, "PCIe: RC1 PERST# de-asserted. \n"));
	mTlmmProtocol->Set(
		MSM_PCIE_GPIO_PERST_PIN,
		1 - MSM_PCIE_GPIO_PERST_PIN_ON
	);

	gBS->Stall(
		PERST_PROPAGATION_DELAY_US_MAX
	);

	/* set max tlp read size */
	WriteRegisterField(
		(void*) MSM_PCIE_RES_DM_CORE, 
		PCIE20_DEVICE_CONTROL_STATUS,
		0x7000, 
		PCIE_TLP_RD_SIZE
	);

	// 9. Enable link training
	DEBUG((EFI_D_ERROR, "PCIe: Enable link training.\n"));
	WriteMask(
		(void*)MSM_PCIE_RES_PARF + PCIE20_PARF_LTSSM,
		0,
		BIT(8)
	);

	// 10. Wait for link to come up
	Status = WaitForLink();
	ASSERT_EFI_ERROR(Status);

	// 11. Configure controller

	Status = gBS->InstallProtocolInterface(
		&ImageHandle,
		&gQcomMsmPCIExpressInitProtocolGuid,
		EFI_NATIVE_INTERFACE,
		&ImageHandle
	);

exit:
	return Status;
}
