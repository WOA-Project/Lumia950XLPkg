/** @file
PCI Host Bridge Library instance for Qualcomm MSM8992/8994

Copyright (c) 2017, Linaro Ltd. All rights reserved.<BR>
Copyright (c) 2018, Bingxing Wang. All rights reserved.<BR>

This program and the accompanying materials are licensed and made available
under the terms and conditions of the BSD License which accompanies this
distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/LKEnvLib.h>
#include "BitOps.h"

#include <IndustryStandard/Pci22.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Chipset/PCIExpress.h>

STATIC UINT8 mCurrentBDF;

STATIC 
BOOLEAN 
CheckPhyStatus(
	VOID
)
{
	if (readl_relaxed(MSM_PCIE_RES_PHY + PCIE_PHY_PCS_STATUS) & BIT(6))
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
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
	DEBUG((EFI_D_ERROR, "PCIe: device ID and vender ID of RC1 are 0x%x.\n", val));

	if (val == PCIE_LINK_DOWN)
	{
		DEBUG((EFI_D_ERROR, "PCIe: The link of RC1 is not really up; device ID and vender ID of RC1 are 0x%x.\n", val));
		return false;
	}

	val = readl_relaxed(MSM_PCIE_RES_DM_CORE + 0x8);
	val = (UINT8) ((val >> 16) >> 8);
	DEBUG((EFI_D_ERROR, "PCIe: Device Class code is 0x%x. \n", val));

	val = readl_relaxed(MSM_PCIE_RES_DM_CORE + 0x8);
	val = (UINT8) (val >> 16);
	DEBUG((EFI_D_ERROR, "PCIe: Device SubClass code is 0x%x. \n", val));

	return true;
}


STATIC
EFI_STATUS
WaitForLink(
	VOID
)
{
	uint32_t link_check_count = 0;
	uint32_t val;

	do {
		DEBUG((EFI_D_ERROR, "Waiting for PCIe link to come up... %d \n", link_check_count));
		gBS->Stall(LINK_UP_TIMEOUT_US_MIN);
		val = readl_relaxed(MSM_PCIE_RES_ELBI + PCIE20_ELBI_SYS_STTS);
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

	ArmDataMemoryBarrier();
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
	WriteMask((void*) MSM_PCIE_RES_DM_CORE + PCIE20_ACK_F_ASPM_CTRL_REG, 0, BIT(15));

	/* Enable AER on RC */
	WriteMask((void*) MSM_PCIE_RES_DM_CORE + PCIE20_BRIDGE_CTRL, 0, BIT(16) | BIT(17));
	WriteMask((void*) MSM_PCIE_RES_DM_CORE + PCIE20_CAP_DEVCTRLSTATUS, 0,
		BIT(3) | BIT(2) | BIT(1) | BIT(0)
	);

	DEBUG((EFI_D_ERROR, "RC1 PCIE20_CAP_DEVCTRLSTATUS:0x%x\n", readl_relaxed(MSM_PCIE_RES_DM_CORE + PCIE20_CAP_DEVCTRLSTATUS)));
}

static inline int CheckAlignment(uint32_t offset)
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
	uint32_t ep_l1sub_ctrl1_offset = 0;
	uint32_t ep_l1sub_cap_reg1_offset = 0;
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

	// L1 Supported
	WriteMask((void*) MSM_PCIE_RES_DM_CORE + PCIE20_CAP_LINKCTRLSTATUS, 0, BIT(1));
	WriteMask((void*) MSM_PCIE_RES_CONF + ep_link_ctrlstts_offset, 0, BIT(1));
	DEBUG((EFI_D_ERROR, "RC1 CAP_LINKCTRLSTATUS:0x%x\n", readl_relaxed(MSM_PCIE_RES_DM_CORE + PCIE20_CAP_LINKCTRLSTATUS)));
	DEBUG((EFI_D_ERROR, "EP1 CAP_LINKCTRLSTATUS:0x%x\n", readl_relaxed(MSM_PCIE_RES_CONF + ep_link_ctrlstts_offset)));

	// L1SS Supported
	current_offset = PCIE_EXT_CAP_OFFSET;
	while (current_offset) 
	{
		if (CheckAlignment(current_offset)) return;

		val = readl_relaxed(MSM_PCIE_RES_CONF + current_offset);
		if ((val & 0xffff) == L1SUB_CAP_ID) 
		{
			ep_l1sub_cap_reg1_offset = current_offset + 0x4;
			ep_l1sub_ctrl1_offset = current_offset + 0x8;
			break;
		}
		current_offset = val >> 20;
	}

	if (!ep_l1sub_ctrl1_offset) 
	{
		DEBUG((EFI_D_ERROR, "RC1 endpoint does not support l1ss registers\n"));
		return;
	}

	val = readl_relaxed(MSM_PCIE_RES_CONF + ep_l1sub_cap_reg1_offset);
	DEBUG((EFI_D_ERROR, "EP's L1SUB_CAPABILITY_REG_1: 0x%x\n", val));
	DEBUG((EFI_D_ERROR, "RC1: ep_l1sub_ctrl1_offset: 0x%x\n", ep_l1sub_ctrl1_offset));
	val &= 0xf;

	WriteRegisterField((void*) MSM_PCIE_RES_DM_CORE, PCIE20_L1SUB_CONTROL1, 0xf, val);
	WriteMask((void*) MSM_PCIE_RES_DM_CORE + PCIE20_DEVICE_CONTROL2_STATUS2, 0, BIT(10));
	WriteRegisterField((void*) MSM_PCIE_RES_CONF, ep_l1sub_ctrl1_offset, 0xf, val);
	WriteMask((void*) MSM_PCIE_RES_CONF + ep_dev_ctrl2stts2_offset, 0, BIT(10));
	DEBUG((EFI_D_ERROR, "RC's L1SUB_CONTROL1:0x%x\n", readl_relaxed(MSM_PCIE_RES_DM_CORE + PCIE20_L1SUB_CONTROL1)));
	DEBUG((EFI_D_ERROR, "RC's DEVICE_CONTROL2_STATUS2:0x%x\n", readl_relaxed(MSM_PCIE_RES_DM_CORE + PCIE20_DEVICE_CONTROL2_STATUS2)));
	DEBUG((EFI_D_ERROR, "EP's L1SUB_CONTROL1:0x%x\n", readl_relaxed(MSM_PCIE_RES_CONF + ep_l1sub_ctrl1_offset)));
	DEBUG((EFI_D_ERROR, "EP's DEVICE_CONTROL2_STATUS2:0x%x\n", readl_relaxed(MSM_PCIE_RES_CONF + ep_dev_ctrl2stts2_offset)));
}

EFI_STATUS
EFIAPI
QcomPciHostBridgeLibConstructor(
	IN EFI_HANDLE       ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
)
{
	EFI_STATUS Status;

	// 1-10. Do nothing. Just check PHY status.
	if (!CheckPhyStatus())
	{
		DEBUG((EFI_D_ERROR, "PCIe RC1 PHY is not ready! \n"));
		ASSERT(FALSE);
	}

	// 11. Set max TLP read size
	DEBUG((EFI_D_ERROR, "PCIe: Set max TLP read size.\n"));
	WriteRegisterField(
		(void*) MSM_PCIE_RES_DM_CORE,
		PCIE20_DEVICE_CONTROL_STATUS,
		0x7000,
		PCIE_TLP_RD_SIZE
	);

	// 12. Enable link training
	DEBUG((EFI_D_ERROR, "PCIe: Enable link training.\n"));
	WriteMask(
		(void*) MSM_PCIE_RES_PARF + PCIE20_PARF_LTSSM,
		0,
		BIT(8)
	);

	// 13. Wait for up to 100ms for the link to come up
	Status = WaitForLink();

	// 14. Configure controller
	ConfigureController();

	// 15. Configure Link State
	ConfigureLinkState();

	// 16. Kick start actual configuration. (Memory BARs?)


	return Status;
}
