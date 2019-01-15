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

#include <Library/PciHostBridgeLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>

#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>

#include <Chipset/PCIExpress.h>

#pragma pack(1)
typedef struct {
	ACPI_HID_DEVICE_PATH     AcpiDevicePath;
	EFI_DEVICE_PATH_PROTOCOL EndDevicePath;
} EFI_PCI_ROOT_BRIDGE_DEVICE_PATH;
#pragma pack ()

STATIC EFI_PCI_ROOT_BRIDGE_DEVICE_PATH mEfiPciRootBridgeDevicePath = 
{
	{
		{
			ACPI_DEVICE_PATH,
			ACPI_DP,
			{
				(UINT8)(sizeof(ACPI_HID_DEVICE_PATH)),
				(UINT8)((sizeof(ACPI_HID_DEVICE_PATH)) >> 8)
			}
		},
		EISA_PNP_ID(0x0A08), // PCI Express
		0
	},
	{
		END_DEVICE_PATH_TYPE,
		END_ENTIRE_DEVICE_PATH_SUBTYPE,
		{
			END_DEVICE_PATH_LENGTH,
			0
		}
	}
};

GLOBAL_REMOVE_IF_UNREFERENCED
CHAR16 *mPciHostBridgeLibAcpiAddressSpaceTypeStr[] = {
	L"Mem", L"I/O", L"Bus"
};

/**
Return all the root bridge instances in an array.

@param Count  Return the count of root bridge instances.
@return All the root bridge instances in an array.

The array should be passed into PciHostBridgeFreeRootBridges()
when it's not used.
**/
PCI_ROOT_BRIDGE *
EFIAPI
PciHostBridgeGetRootBridges(
	UINTN *Count
)
{
	PCI_ROOT_BRIDGE     *RootBridge = 0;
	PCI_ROOT_BRIDGE     *CurRootBridge = 0;

	// We have only one root bridge on MSM8992/8994
	*Count = 1;
	RootBridge = AllocateZeroPool(*Count * sizeof(PCI_ROOT_BRIDGE));
	CurRootBridge = RootBridge;

	CurRootBridge->Segment = 0;
	CurRootBridge->Supports = 0;
	CurRootBridge->Attributes = CurRootBridge->Supports;
	CurRootBridge->DmaAbove4G = FALSE;
	CurRootBridge->AllocationAttributes = EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM;

	CurRootBridge->Bus.Base = 0x0;
	CurRootBridge->Bus.Limit = 0xff;
	CurRootBridge->Io.Base = 0;
	CurRootBridge->Io.Limit = 0;
	CurRootBridge->Mem.Base = 0xf8c0;
	CurRootBridge->Mem.Limit = 0xf8e0;
	CurRootBridge->MemAbove4G.Base = MAX_UINT64;
	CurRootBridge->MemAbove4G.Limit = 0;

	//
	// No separate ranges for prefetchable and non-prefetchable BARs
	//
	CurRootBridge->PMem.Base = 0xf8e1;
	CurRootBridge->PMem.Limit = 0;
	CurRootBridge->PMemAbove4G.Base = MAX_UINT64;
	CurRootBridge->PMemAbove4G.Limit = 0;

	CurRootBridge->NoExtendedConfigSpace = FALSE;
	CurRootBridge->DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) &mEfiPciRootBridgeDevicePath;

	*Count = 0;

	return RootBridge;
}

/**
Free the root bridge instances array returned from PciHostBridgeGetRootBridges().

@param Bridges The root bridge instances array.
@param Count   The count of the array.

**/
VOID
EFIAPI
PciHostBridgeFreeRootBridges(
	PCI_ROOT_BRIDGE *Bridges,
	UINTN           Count
)
{
	FreePool(Bridges);
}


/**
Inform the platform that the resource conflict happens.

@param HostBridgeHandle Handle of the Host Bridge.
@param Configuration    Pointer to PCI I/O and PCI memory resource
descriptors. The Configuration contains the resources
for all the root bridges. The resource for each root
bridge is terminated with END descriptor and an
additional END is appended indicating the end of the
entire resources. The resource descriptor field
values follow the description in
EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL
.SubmitResources().
**/
VOID
EFIAPI
PciHostBridgeResourceConflict(
	EFI_HANDLE                        HostBridgeHandle,
	VOID                              *Configuration
)
{
	EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Descriptor;
	UINTN                             RootBridgeIndex;
	DEBUG((EFI_D_ERROR, "PciHostBridge: Resource conflict happens!\n"));

	RootBridgeIndex = 0;
	Descriptor = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Configuration;
	while (Descriptor->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR) 
	{
		DEBUG((EFI_D_ERROR, "RootBridge[%d]:\n", RootBridgeIndex++));
		for (; Descriptor->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR; Descriptor++) 
		{
			ASSERT(Descriptor->ResType <
				(sizeof(mPciHostBridgeLibAcpiAddressSpaceTypeStr) /
				sizeof(mPciHostBridgeLibAcpiAddressSpaceTypeStr[0])
			));

			DEBUG((EFI_D_ERROR, " %s: Length/Alignment = 0x%lx / 0x%lx\n",
				mPciHostBridgeLibAcpiAddressSpaceTypeStr[Descriptor->ResType],
				Descriptor->AddrLen, Descriptor->AddrRangeMax
			));

			if (Descriptor->ResType == ACPI_ADDRESS_SPACE_TYPE_MEM) 
			{
				DEBUG((EFI_D_ERROR, "     Granularity/SpecificFlag = %ld / %02x%s\n",
					Descriptor->AddrSpaceGranularity, Descriptor->SpecificFlag,
					((Descriptor->SpecificFlag &
						EFI_ACPI_MEMORY_RESOURCE_SPECIFIC_FLAG_CACHEABLE_PREFETCHABLE
						) != 0) ? L" (Prefetchable)" : L""
				));
			}
		}

		//
		// Skip the END descriptor for root bridge
		//
		ASSERT(Descriptor->Desc == ACPI_END_TAG_DESCRIPTOR);
		Descriptor = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) (
			(EFI_ACPI_END_TAG_DESCRIPTOR *) Descriptor + 1
		);
	}
}