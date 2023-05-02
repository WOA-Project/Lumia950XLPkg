/*++

Copyright (c) Microsoft Corporation

SPDX-License-Identifier: BSD-2-Clause-Patent

Module Name:

    uefiext.h

Abstract:

    Global definitions for the UEFI debug extensions.

--*/

#include "dbgexts.h"

//
// Generic definitions
//

#define PAGE_SIZE (0x1000)
#define PAGE_ALIGN_DOWN(_ptr) (_ptr & ~(PAGE_SIZE - 1))

//
// EFI environment information.
//

typedef enum _UEFI_ENV {
    PEI,
    DXE,
    MM,
    UNKNOWN
} UEFI_ENV;

extern UEFI_ENV gUefiEnv;

//
// EFI tables structions and functions.
//

typedef enum _EFI_TABLE {
    HobList = 0,
    // Add more tables here as needed, and add guid to same offset below.
} EFI_TABLE;

#define EFI_TABLE_GUIDS \
    { \
        { 0x7739F24C, 0x93D7, 0x11D4, { 0x9A, 0x3A, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D }} /* Hoblist */ \
    }

ULONG64 GetTableAddress(EFI_TABLE Table);
