/** @file
  PCI Host Bridge Library instance for pci-ecam-generic DT nodes

  Copyright (c) 2019, Linaro Ltd. All rights reserved

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PciHostBridgeLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <PiDxe.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>

#pragma pack(1)
typedef struct {
  ACPI_HID_DEVICE_PATH     AcpiDevicePath;
  EFI_DEVICE_PATH_PROTOCOL EndDevicePath;
} EFI_PCI_ROOT_BRIDGE_DEVICE_PATH;
#pragma pack ()

STATIC EFI_PCI_ROOT_BRIDGE_DEVICE_PATH mEfiPciRootBridgeDevicePath = {
  {
    {
      ACPI_DEVICE_PATH,
      ACPI_DP,
      {
        (UINT8) (sizeof(ACPI_HID_DEVICE_PATH)),
        (UINT8) ((sizeof(ACPI_HID_DEVICE_PATH)) >> 8)
      }
    },
    EISA_PNP_ID(0x0A03),
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

STATIC EFI_PCI_ROOT_BRIDGE_DEVICE_PATH mEfiPciRootBridgeDevicePath2 = {
  {
    {
      ACPI_DEVICE_PATH,
      ACPI_DP,
      {
        (UINT8) (sizeof(ACPI_HID_DEVICE_PATH)),
        (UINT8) ((sizeof(ACPI_HID_DEVICE_PATH)) >> 8)
      }
    },
    EISA_PNP_ID(0x0A03),
    1
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

STATIC PCI_ROOT_BRIDGE mRootBridgeCollection[2] = { 
  {
    /* UINT32 Segment; Segment number */
    0,

    /* UINT64 Supports; Supported attributes */
    0,

    /* UINT64 Attributes; Initial attributes */
    0,

    /* BOOLEAN DmaAbove4G; DMA above 4GB memory */
    FALSE,

    /* BOOLEAN NoExtendedConfigSpace; When FALSE, the root bridge supports
      Extended (4096-byte) Configuration Space.  When TRUE, the root bridge
      supports 256-byte Configuration Space only. */
    FALSE,

    /* BOOLEAN ResourceAssigned; Resource assignment status of the root bridge.
      Set to TRUE if Bus/IO/MMIO resources for root bridge have been assigned */
    FALSE,

    /* UINT64 AllocationAttributes; Allocation attributes. */
    EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM,

    {
      /* PCI_ROOT_BRIDGE_APERTURE Bus; Bus aperture which can be used by the
        * root bridge. */
      0, 255
    },

    /* PCI_ROOT_BRIDGE_APERTURE Io; IO aperture which can be used by the root
      bridge */
    { MAX_UINT64, 0 },

    /* PCI_ROOT_BRIDGE_APERTURE Mem; MMIO aperture below 4GB which can be used by
      the root bridge
      (gEfiMdePkgTokenSpaceGuid.PcdPciMmio32Translation as 0x0) */
    {
      0xFF000000,
      0xFF000000 + 0x00800000 - 1,
    },

    /* PCI_ROOT_BRIDGE_APERTURE MemAbove4G; MMIO aperture above 4GB which can be
      used by the root bridge.
      (gEfiMdePkgTokenSpaceGuid.PcdPciMmio64Translation as 0x0)
      QC platform do not have it on MSM8994 */
    { MAX_UINT64, 0 },

    /* PCI_ROOT_BRIDGE_APERTURE PMem; Prefetchable MMIO aperture below 4GB which
      can be used by the root bridge.
      In our case, there are no separate ranges for prefetchable and
      non-prefetchable BARs */
    { MAX_UINT64, 0 },

    /* PCI_ROOT_BRIDGE_APERTURE PMemAbove4G; Prefetchable MMIO aperture above 4GB
      which can be used by the root bridge. */
    { MAX_UINT64, 0 },
    /* EFI_DEVICE_PATH_PROTOCOL *DevicePath; Device path. */
    (EFI_DEVICE_PATH_PROTOCOL *)&mEfiPciRootBridgeDevicePath,
  },
  {
    /* UINT32 Segment; Segment number */
    0,

    /* UINT64 Supports; Supported attributes */
    0,

    /* UINT64 Attributes; Initial attributes */
    0,

    /* BOOLEAN DmaAbove4G; DMA above 4GB memory */
    FALSE,

    /* BOOLEAN NoExtendedConfigSpace; When FALSE, the root bridge supports
      Extended (4096-byte) Configuration Space.  When TRUE, the root bridge
      supports 256-byte Configuration Space only. */
    FALSE,

    /* BOOLEAN ResourceAssigned; Resource assignment status of the root bridge.
      Set to TRUE if Bus/IO/MMIO resources for root bridge have been assigned */
    FALSE,

    /* UINT64 AllocationAttributes; Allocation attributes. */
    EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM,

    {
      /* PCI_ROOT_BRIDGE_APERTURE Bus; Bus aperture which can be used by the
        * root bridge. */
      0, 255
    },

    /* PCI_ROOT_BRIDGE_APERTURE Io; IO aperture which can be used by the root
      bridge */
    { MAX_UINT64, 0 },

    /* PCI_ROOT_BRIDGE_APERTURE Mem; MMIO aperture below 4GB which can be used by
      the root bridge
      (gEfiMdePkgTokenSpaceGuid.PcdPciMmio32Translation as 0x0) */
    {
      0xF8800000,
      0xF8800000 + 0x00800000 - 1,
    },

    /* PCI_ROOT_BRIDGE_APERTURE MemAbove4G; MMIO aperture above 4GB which can be
      used by the root bridge.
      (gEfiMdePkgTokenSpaceGuid.PcdPciMmio64Translation as 0x0)
      QC platform do not have it on MSM8994 */
    { MAX_UINT64, 0 },

    /* PCI_ROOT_BRIDGE_APERTURE PMem; Prefetchable MMIO aperture below 4GB which
      can be used by the root bridge.
      In our case, there are no separate ranges for prefetchable and
      non-prefetchable BARs */
    { MAX_UINT64, 0 },

    /* PCI_ROOT_BRIDGE_APERTURE PMemAbove4G; Prefetchable MMIO aperture above 4GB
      which can be used by the root bridge. */
    { MAX_UINT64, 0 },
    /* EFI_DEVICE_PATH_PROTOCOL *DevicePath; Device path. */
    (EFI_DEVICE_PATH_PROTOCOL *)&mEfiPciRootBridgeDevicePath2,
  },
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
PciHostBridgeGetRootBridges (
  UINTN *Count
  )
{
  *Count = 1;
  return &mRootBridgeCollection[0];
}

/**
  Free the root bridge instances array returned from
  PciHostBridgeGetRootBridges().

  @param Bridges The root bridge instances array.
  @param Count   The count of the array.
**/
VOID
EFIAPI
PciHostBridgeFreeRootBridges (
  PCI_ROOT_BRIDGE *Bridges,
  UINTN           Count
  )
{
  ASSERT (Count == 1);
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
PciHostBridgeResourceConflict (
  EFI_HANDLE                        HostBridgeHandle,
  VOID                              *Configuration
  )
{
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Descriptor;
  UINTN                             RootBridgeIndex;
  DEBUG ((DEBUG_ERROR, "PciHostBridge: Resource conflict happens!\n"));

  RootBridgeIndex = 0;
  Descriptor = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Configuration;
  while (Descriptor->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR) {
    DEBUG ((DEBUG_ERROR, "RootBridge[%d]:\n", RootBridgeIndex++));
    for (; Descriptor->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR; Descriptor++) {
      ASSERT (Descriptor->ResType <
               ARRAY_SIZE(mPciHostBridgeLibAcpiAddressSpaceTypeStr));
      DEBUG ((DEBUG_ERROR, " %s: Length/Alignment = 0x%lx / 0x%lx\n",
              mPciHostBridgeLibAcpiAddressSpaceTypeStr[Descriptor->ResType],
              Descriptor->AddrLen, Descriptor->AddrRangeMax
              ));
      if (Descriptor->ResType == ACPI_ADDRESS_SPACE_TYPE_MEM) {
        DEBUG ((DEBUG_ERROR, "     Granularity/SpecificFlag = %ld / %02x%s\n",
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
    ASSERT (Descriptor->Desc == ACPI_END_TAG_DESCRIPTOR);
    Descriptor = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)(
                   (EFI_ACPI_END_TAG_DESCRIPTOR *)Descriptor + 1
                   );
  }
}
