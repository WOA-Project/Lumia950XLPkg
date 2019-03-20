/** @file
  Head file for BDS Platform specific code

  Copyright (C) 2015-2016, Red Hat, Inc.
  Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.
  Copyright (c) 2016, Linaro Ltd. All rights reserved.
  Copyright (c), 2017, Andrei Warkentin <andrey.warkentin@gmail.com>

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _PLATFORM_BM_H_
#define _PLATFORM_BM_H_

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

/* Build-time generated ReleaseInfo.h will override the default one */
#include <Resources/ReleaseStampStub.h>
// Must come in order
#include <Resources/ReleaseInfo.h>

/**
  Use SystemTable Conout to stop video based Simple Text Out consoles from
  going to the video device. Put up LogoFile on every video device that is a
  console.

  @param[in]  LogoFile   File name of logo to display on the center of the
                         screen.

  @retval EFI_SUCCESS     ConsoleControl has been flipped to graphics and logo
                          displayed.
  @retval EFI_UNSUPPORTED Logo not found
**/
EFI_STATUS
EnableQuietBoot(IN EFI_GUID *LogoFile);

/**
  Use SystemTable Conout to turn on video based Simple Text Out consoles. The
  Simple Text Out screens will now be synced up with all non video output
  devices

  @retval EFI_SUCCESS     UGA devices are back in text mode and synced up.
**/
EFI_STATUS
DisableQuietBoot(VOID);

#endif // _PLATFORM_BM_H_
