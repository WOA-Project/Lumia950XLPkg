/** @file
        Copying Functions for <Library/minstring.h>.

        Copyright (c) 2010 - 2011, Intel Corporation. All rights reserved.<BR>
        This program and the accompanying materials are licensed and made
available under the terms and conditions of the BSD License that accompanies
this distribution. The full text of the license may be found at
        http://opensource.org/licenses/bsd-license.php.

        THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
        WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR
IMPLIED.
**/

#ifndef __LV_COMPAT_H__
#define __LV_COMPAT_H__

#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/TimerLib.h>


CHAR8 *EFIAPI AsciiStrCpy(OUT CHAR8 *Destination, IN CONST CHAR8 *Source);

#endif
