/*++

Copyright (c) Microsoft Corporation

SPDX-License-Identifier: BSD-2-Clause-Patent

Module Name:

    tables.cpp

Abstract:

    This file contains debug commands enumerating EFI tables.

--*/

#include "uefiext.h"

GUID TableGuids[] = EFI_TABLE_GUIDS;

ULONG64
GetTableAddress(EFI_TABLE Table)
{

    ULONG64 StPtrAddr;
    ULONG64 SystemTableAddr;
    ULONG64 NumTables;
    ULONG64 ConfigTables;
    ULONG64 TableAddr;
    ULONG64 Result;
    ULONG64 ConfigTableSize;
    GUID TableGuid;
    ULONG64 i;

    if (gUefiEnv == DXE) {
        StPtrAddr = GetExpression("gST");
        if (StPtrAddr == NULL) {
            dprintf("Failed to find global system table!\n");
            return 0;
        }

        if (!ReadPointer(StPtrAddr, &SystemTableAddr)) {
            dprintf("Failed to find global system table!\n");
            return 0;
        }

        GetFieldValue(SystemTableAddr, "EFI_SYSTEM_TABLE", "NumberOfTableEntries", NumTables);
        GetFieldValue(SystemTableAddr, "EFI_SYSTEM_TABLE", "ConfigurationTable", ConfigTables);
        ConfigTableSize = GetTypeSize("EFI_CONFIGURATION_TABLE");

        //
        // Check the number of tables in case something is wrong.
        //

        if (NumTables > 100) {
            dprintf("Found too many system tables! (%lld) \n", NumTables);
            return 0;
        }

        for (i = 0; i < NumTables; i++) {
            TableAddr = ConfigTables + (i * ConfigTableSize);
            GetFieldValue(TableAddr, "EFI_CONFIGURATION_TABLE", "VendorGuid", TableGuid);
            if (TableGuid == TableGuids[Table]) {
                GetFieldValue(TableAddr, "EFI_CONFIGURATION_TABLE", "VendorTable", Result);
                return Result;
            }
        }

    }
    else {
        dprintf("Not supported for this environment!\n");
    }

    return 0;
}
