/*++

    Copyright (c) Microsoft Corporation

    SPDX-License-Identifier: BSD-2-Clause-Patent

Module Name:

    memory.cpp

Abstract:

    This file contains debug commands for memory operations.

--*/

#include "uefiext.h"

//
// **************************************************************  Definitions
//

PCSTR MemoryTypeString[] = {
    "EfiReservedMemoryType",
    "EfiLoaderCode",
    "EfiLoaderData",
    "EfiBootServicesCode",
    "EfiBootServicesData",
    "EfiRuntimeServicesCode",
    "EfiRuntimeServicesData",
    "EfiConventionalMemory",
    "EfiUnusableMemory",
    "EfiACPIReclaimMemory",
    "EfiACPIMemoryNVS",
    "EfiMemoryMappedIO",
    "EfiMemoryMappedIOPortSpace",
    "EfiPalCode",
    "EfiPersistentMemory"
};

#define MEMORY_TYPE_COUNT (sizeof(MemoryTypeString) / sizeof(MemoryTypeString[0]))

PSTR HobTypes[] = {
    NULL,                                   // 0x0000
    "EFI_HOB_HANDOFF_INFO_TABLE",           // 0x0001
    "EFI_HOB_MEMORY_ALLOCATION",            // 0x0002
    "EFI_HOB_RESOURCE_DESCRIPTOR",          // 0x0003
    "EFI_HOB_GUID_TYPE",                    // 0x0004
    "EFI_HOB_FIRMWARE_VOLUME",              // 0x0005
    "EFI_HOB_CPU",                          // 0x0006
    "EFI_HOB_MEMORY_POOL",                  // 0x0007
    NULL,                                   // 0x0008
    "EFI_HOB_FIRMWARE_VOLUME2",             // 0x0009
    NULL,                                   // 0x000A
    "EFI_HOB_UEFI_CAPSULE",                 // 0x000B
    "EFI_HOB_FIRMWARE_VOLUME3"              // 0x000C
};

#define HOB_TYPE_COUNT (sizeof(HobTypes) / sizeof(HobTypes[0]))

//
// *******************************************************  External Functions
//

HRESULT CALLBACK
memorymap(PDEBUG_CLIENT4 Client, PCSTR args)
{
    ULONG64 Address;
    ULONG64 HeadAddress;
    ULONG64 MemoryEntry;
    ULONG ListEntryOffset;
    ULONG Type;
    UINT64 Start;
    UINT64 End;
    UINT64 Pages;
    UINT64 Attribute;
    ULONG64 TotalMemory = 0;
    ULONG64 TypeSize[MEMORY_TYPE_COUNT] = { 0 };
    INIT_API();

    UNREFERENCED_PARAMETER(args);

    if (gUefiEnv != DXE) {
        dprintf("Only supported for DXE!\n");
        return ERROR_NOT_SUPPORTED;
    }

    GetFieldOffset("MEMORY_MAP", "Link", &ListEntryOffset);
    HeadAddress = GetExpression("&gMemoryMap");
    if (HeadAddress == NULL) {
        dprintf("Failed to find gMemoryMap!\n");
        return ERROR_NOT_FOUND;
    }

    dprintf("    Start             End               Pages             Attributes        MemoryType   \n");
    dprintf("-------------------------------------------------------------------------------------------------------\n");
    for (GetFieldValue(HeadAddress, "_LIST_ENTRY", "ForwardLink", Address);
        Address != HeadAddress;
        GetFieldValue(Address, "_LIST_ENTRY", "ForwardLink", Address)) {

        if (Address == NULL) {
            dprintf("NULL address found in list!\n");
            return ERROR_NOT_FOUND;
        }

        MemoryEntry = Address - ListEntryOffset;
        GetFieldValue(MemoryEntry, "MEMORY_MAP", "Type", Type);
        GetFieldValue(MemoryEntry, "MEMORY_MAP", "Start", Start);
        GetFieldValue(MemoryEntry, "MEMORY_MAP", "End", End);
        GetFieldValue(MemoryEntry, "MEMORY_MAP", "Attribute", Attribute);
        Pages = ((End + 1) - Start) / PAGE_SIZE;

        dprintf("    %016I64x  %016I64x  %16I64x  %016I64x  %-2d (%s)\n",
            Start,
            End,
            Pages,
            Attribute,
            Type,
            Type < MEMORY_TYPE_COUNT ? MemoryTypeString[Type] : "Unknown");

        //
        // Memory size tracking.
        //

        TotalMemory += (End - Start + 1);
        if (Type < MEMORY_TYPE_COUNT) {
            TypeSize[Type] += (End - Start + 1);
        }
    }
    dprintf("-------------------------------------------------------------------------------------------------------\n");
    for (Type = 0; Type < MEMORY_TYPE_COUNT; Type++) {
        dprintf("    %-30s %16I64x\n", MemoryTypeString[Type], TypeSize[Type]);
    }
    dprintf("\n    %-30s %16I64x\n", "Total", TotalMemory);

    dprintf("-------------------------------------------------------------------------------------------------------\n");

    EXIT_API();
    return S_OK;
}

HRESULT CALLBACK
hobs(PDEBUG_CLIENT4 Client, PCSTR args)
{
    ULONG64 HobAddr;
    UINT16 HobType;
    UINT16 HobLength;
    PSTR TypeString;

    INIT_API();

    //
    // Collect the hobs in the environment specific way.
    //

    if (gUefiEnv == DXE) {
        if (GetExpressionEx(args, &HobAddr, &args) == FALSE) {
            HobAddr = GetTableAddress(HobList);
        }

        if (HobAddr == 0) {
            dprintf("Hob list not found!\n");
            return ERROR_NOT_FOUND;
        }

        dprintf("Enumerating Hob list at 0x%I64x\n\n", HobAddr);
        dprintf("    Address             Length  Type\n");
        dprintf("-------------------------------------------------------------------\n");
        do {
            GetFieldValue(HobAddr, "EFI_HOB_GENERIC_HEADER", "HobType", HobType);
            GetFieldValue(HobAddr, "EFI_HOB_GENERIC_HEADER", "HobLength", HobLength);

            dprintf("    %016I64x    %04x    (0x%x) - ",
                HobAddr,
                HobLength,
                HobType,
                HobType < HOB_TYPE_COUNT ? HobTypes[HobType] : "UKNOWN");

            //
            // Handle enumerations if possible
            //

            TypeString = NULL;
            if (HobType < HOB_TYPE_COUNT) {
                TypeString = HobTypes[HobType];
            }

            if (TypeString == NULL) {
                TypeString = "EFI_HOB_GENERIC_HEADER";
            }

            g_ExtControl->ControlledOutput(
                DEBUG_OUTCTL_AMBIENT_DML,
                DEBUG_OUTPUT_NORMAL,
                "<exec cmd=\"dt %s %016I64x\">%s</exec> ",
                TypeString,
                HobAddr,
                TypeString);

            dprintf("\n");

            HobAddr += HobLength;
        } while ((HobType != 0xFFFF) && (HobLength != 0));

    }
    else {
        dprintf("Not supported for this environment!\n");
        return ERROR_NOT_SUPPORTED;
    }

    EXIT_API();
    return S_OK;
}
