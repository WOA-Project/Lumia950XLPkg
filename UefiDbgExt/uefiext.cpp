/*++

Copyright (c) Microsoft Corporation

SPDX-License-Identifier: BSD-2-Clause-Patent

Module Name:

    uefiext.cpp

Abstract:

    This file contains core UEFI debug commands.

--*/

#include "uefiext.h"

UEFI_ENV gUefiEnv = DXE;

HRESULT
NotifyOnTargetAccessible(PDEBUG_CONTROL Control)
{

    //
    // Attempt to determine what environment the debugger is in.
    //

    return S_OK;
}

HRESULT CALLBACK
setenv(PDEBUG_CLIENT4 Client, PCSTR args)
{
    INIT_API();

    if (_stricmp(args, "PEI") == 0) {
        gUefiEnv = PEI;
    }
    else if (_stricmp(args, "DXE") == 0) {
        gUefiEnv = DXE;
    }
    else if (_stricmp(args, "MM") == 0) {
        gUefiEnv = MM;
    }
    else {
        dprintf("Unknown environment type! Supported types: PEI, DXE, MM");
    }

    EXIT_API();
    return S_OK;
}

HRESULT CALLBACK
help(PDEBUG_CLIENT4 Client, PCSTR args)
{
    INIT_API();

    UNREFERENCED_PARAMETER(args);

    dprintf("Help for uefiext.dll\n"
        "  help                - Shows this help\n"
        "  findall             - Attempts to detect environment and load all modules\n"
        "  findmodule          - Find the currently running module\n"
        "  memorymap           - Prints the current memory map\n"
        "  loadmodules         - Find and loads symbols for all modules in the debug list\n"
        "  setenv              - Set the extensions environment mode\n"
        "  hobs                - Enumerates the hand off blocks\n"
    );

    EXIT_API();

    return S_OK;
}

VOID
PrintGuid(GUID Guid) {
    dprintf("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
        Guid.Data1,
        Guid.Data2,
        Guid.Data3,
        Guid.Data4[0], Guid.Data4[1],
        Guid.Data4[2], Guid.Data4[3], Guid.Data4[4], Guid.Data4[5], Guid.Data4[6], Guid.Data4[7]);
}
