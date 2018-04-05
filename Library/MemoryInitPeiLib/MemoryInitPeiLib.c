/** @file
*
*  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include <PiPei.h>

#include <Library/ArmMmuLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>

// This varies by device
#include <Configuration/DeviceMemoryMap.h>

extern UINT64 mSystemMemoryEnd;

VOID
BuildMemoryTypeInformationHob
(
    VOID
);

STATIC
VOID
InitMmu
(
    IN ARM_MEMORY_REGION_DESCRIPTOR  *MemoryTable
)
{

    VOID                          *TranslationTableBase;
    UINTN                         TranslationTableSize;
    RETURN_STATUS                 Status;

    // Note: Because we called PeiServicesInstallPeiMemory() before 
    // to call InitMmu() the MMU Page Table resides in
    // RAM (even at the top of DRAM as it is the first permanent memory allocation)
    Status = ArmConfigureMmu(
      MemoryTable, 
      &TranslationTableBase, 
      &TranslationTableSize
    );

    if (EFI_ERROR(Status))
    {
        DEBUG((EFI_D_ERROR, "Error: Failed to enable MMU: %r\n", Status));
    }
}

STATIC
VOID
AddAndReserved
(
    PARM_MEMORY_REGION_DESCRIPTOR_EXTENDED Desc
)
{
    BuildResourceDescriptorHob(
        EFI_RESOURCE_SYSTEM_MEMORY,
        EFI_RESOURCE_ATTRIBUTE_PRESENT |
        EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
        EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
        EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
        EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
        EFI_RESOURCE_ATTRIBUTE_TESTED,
        Desc->PhysicalBase,
        Desc->Length
    );

    BuildMemoryAllocationHob(
        Desc->PhysicalBase,
        Desc->Length,
        EfiReservedMemoryType
    );
}

STATIC
VOID
AddUsable
(
    PARM_MEMORY_REGION_DESCRIPTOR_EXTENDED Desc
)
{
    BuildResourceDescriptorHob(
      EFI_RESOURCE_SYSTEM_MEMORY,
      EFI_RESOURCE_ATTRIBUTE_PRESENT |
      EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
      EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
      EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_TESTED,
      Desc->PhysicalBase,
      Desc->Length
    );
}

STATIC
VOID
AddAndMmio(
    PARM_MEMORY_REGION_DESCRIPTOR_EXTENDED Desc
)
{
    BuildResourceDescriptorHob(
        EFI_RESOURCE_SYSTEM_MEMORY,
        (EFI_RESOURCE_ATTRIBUTE_PRESENT    |
        EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
        EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
        EFI_RESOURCE_ATTRIBUTE_TESTED),
        Desc->PhysicalBase,
        Desc->Length
    );

    BuildMemoryAllocationHob(
        Desc->PhysicalBase,
        Desc->Length,
        EfiMemoryMappedIO
    );
}

/*++

Routine Description:



Arguments:

  FileHandle  - Handle of the file being invoked.
  PeiServices - Describes the list of possible PEI Services.

Returns:

  Status -  EFI_SUCCESS if the boot mode could be set

--*/
EFI_STATUS
EFIAPI
MemoryPeim
(
    IN EFI_PHYSICAL_ADDRESS               UefiMemoryBase,
    IN UINT64                             UefiMemorySize
)
{

    PARM_MEMORY_REGION_DESCRIPTOR_EXTENDED MemoryDescriptorEx = gDeviceMemoryDescriptor;
    ARM_MEMORY_REGION_DESCRIPTOR MemoryDescriptor[MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT];
    UINTN Index = 0;

    // Ensure PcdSystemMemorySize has been set
    ASSERT (PcdGet64 (PcdSystemMemorySize) != 0);

    // Run through each memory descriptor
    while (MemoryDescriptorEx->Length != 0)
    {
        switch (MemoryDescriptorEx->HobOption)
        {
            case AddMem:
                AddUsable(MemoryDescriptorEx);
                break;
            case AddDev:
                AddAndMmio(MemoryDescriptorEx);
                break;
            case AddReserved:
                AddAndReserved(MemoryDescriptorEx);
                break;
            case NoHob:
            default:
                goto update;
        }

    update:
        ASSERT(Index < MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT);

        MemoryDescriptor[Index].PhysicalBase = MemoryDescriptorEx->PhysicalBase;
        MemoryDescriptor[Index].VirtualBase = MemoryDescriptorEx->VirtualBase;
        MemoryDescriptor[Index].Length = MemoryDescriptorEx->Length;
        MemoryDescriptor[Index].Attributes = MemoryDescriptorEx->Attributes;

        Index++;
        MemoryDescriptorEx++;
    }

    // Last one (terminator)
    ASSERT(Index < MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT);
    MemoryDescriptor[Index].PhysicalBase = 0;
    MemoryDescriptor[Index].VirtualBase = 0;
    MemoryDescriptor[Index].Length = 0;
    MemoryDescriptor[Index].Attributes = 0;

    // Build Memory Allocation Hob
    InitMmu(MemoryDescriptor);

    if (FeaturePcdGet (PcdPrePiProduceMemoryTypeInformationHob)) 
    {
        // Optional feature that helps prevent EFI memory map fragmentation.
        BuildMemoryTypeInformationHob();
    }

    return EFI_SUCCESS;
}