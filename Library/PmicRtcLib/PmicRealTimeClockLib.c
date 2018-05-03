/** @file
  Implement EFI RealTimeClock runtime services for Qualcomm PM8916

  Copyright (c) 2015, Linaro Ltd. All rights reserved.<BR>
  Copyright (c) 2018, Bingxing Wang. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <PiDxe.h>

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Library/LKEnvLib.h>
#include <Library/IoLib.h>
#include <Library/dload_util.h>
#include <Library/QcomPm8x41Lib.h>
#include <Platform/iomap.h>
#include <Chipset/reboot.h>

/* RTC Register offsets from RTC CTRL REG */
#define PM8916_RTC_CONTROL_ADDR	0x6046
#define PM8916_RTC_READ_ADDR 0x6048
#define NUM_8_BIT_RTC_REGS 0x04
#define PM8xxx_RTC_ENABLE	(1UL << 7)

#define PM8916_PON_PBL_STATUS 0x807
#define PM8916_PON_PBL_STATUS_XVDD_RB_OCCURRED (1UL << 6)

/* Default time offset: Thursday, March 15, 2018 12:00:00 AM GMT-07:00 */
#define DEFAULT_TIME_OFFSET 1521097200

/* Local definitions */
BOOLEAN mDeviceReady = FALSE;
UINTN mRtcOffset = DEFAULT_TIME_OFFSET;
STATIC EFI_EVENT mRtcVirtualAddrChangeEvent;

/**
Fixup internal data so that EFI can be call in virtual mode.
Call the passed in Child Notify event and convert any pointers in
lib to virtual mode.

@param[in]    Event   The Event that is being processed
@param[in]    Context Event Context
**/
VOID
EFIAPI
LibRtcVirtualNotifyEvent(
	IN EFI_EVENT        Event,
	IN VOID             *Context
)
{
	//
	// Only needed if you are going to support the OS calling RTC functions in virtual mode.
	// You will need to call EfiConvertPointer (). To convert any stored physical addresses
	// to virtual address. After the OS transitions to calling in virtual mode, all future
	// runtime calls will be made in virtual mode.
	//
	mDeviceReady = FALSE;
	return;
}

/**
  Converts Epoch seconds (elapsed since 1970 JANUARY 01, 00:00:00 UTC) to EFI_TIME
 **/
STATIC
VOID
EpochToEfiTime (
  IN  UINTN     EpochSeconds,
  OUT EFI_TIME  *Time
  )
{
	  UINTN         a;
	  UINTN         b;
	  UINTN         c;
	  UINTN         d;
	  UINTN         g;
	  UINTN         j;
	  UINTN         m;
	  UINTN         y;
	  UINTN         da;
	  UINTN         db;
	  UINTN         dc;
	  UINTN         dg;
	  UINTN         hh;
	  UINTN         mm;
	  UINTN         ss;
	  UINTN         J;

	  J  = (EpochSeconds / 86400) + 2440588;
	  j  = J + 32044;
	  g  = j / 146097;
	  dg = j % 146097;
	  c  = (((dg / 36524) + 1) * 3) / 4;
	  dc = dg - (c * 36524);
	  b  = dc / 1461;
	  db = dc % 1461;
	  a  = (((db / 365) + 1) * 3) / 4;
	  da = db - (a * 365);
	  y  = (g * 400) + (c * 100) + (b * 4) + a;
	  m  = (((da * 5) + 308) / 153) - 2;
	  d  = da - (((m + 4) * 153) / 5) + 122;

	  Time->Year  = y - 4800 + ((m + 2) / 12);
	  Time->Month = ((m + 2) % 12) + 1;
	  Time->Day   = d + 1;

	  ss = EpochSeconds % 60;
	  a  = (EpochSeconds - ss) / 60;
	  mm = a % 60;
	  b = (a - mm) / 60;
	  hh = b % 24;

	  Time->Hour        = hh;
	  Time->Minute      = mm;
	  Time->Second      = ss;
	  Time->Nanosecond  = 0;

}

/**
  Returns the current time and date information, and the time-keeping capabilities
  of the hardware platform.

  @param  Time                  A pointer to storage to receive a snapshot of the current time.
  @param  Capabilities          An optional pointer to a buffer to receive the real time clock
                                device's capabilities.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER Time is NULL.
  @retval EFI_DEVICE_ERROR      The time could not be retrieved due to hardware error.

**/
EFI_STATUS
EFIAPI
LibGetTime (
  OUT EFI_TIME                *Time,
  OUT EFI_TIME_CAPABILITIES  *Capabilities
  )
{
	  ASSERT (Time != NULL);

	  if (!mDeviceReady) return EFI_NOT_READY; 

	  EFI_STATUS Status = EFI_SUCCESS;
	  UINTN secs = 0;
	  UINT8 value[NUM_8_BIT_RTC_REGS];
  
	  for (UINTN i = 0; i < 4; i++)
	  {
		value[i] = gPm8x41->pm8x41_reg_read(PM8916_RTC_READ_ADDR + i);
	  }

	  if (value[0] < 0)
	  {
		DEBUG((EFI_D_ERROR, "PM8916 RTC reported error.\n"));
		return EFI_DEVICE_ERROR;
	  }

	  // Convert RTC epoch time
	  secs = value[0] | (value[1] << 8) | (value[2] << 16) | (value[3] << 24);
	  // Remember our offset
	  secs = secs + mRtcOffset;

	  // Convert UNIX epoch to EFI time.
	  EpochToEfiTime(secs, Time);

	  return Status;
}

/**
  Sets the current local time and date information.

  @param  Time                  A pointer to the current time.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The time could not be set due due to hardware error.

**/
EFI_STATUS
EFIAPI
LibSetTime (
  IN EFI_TIME                *Time
  )
{
	return EFI_DEVICE_ERROR;
}


/**
  Returns the current wakeup alarm clock setting.

  @param  Enabled               Indicates if the alarm is currently enabled or disabled.
  @param  Pending               Indicates if the alarm signal is pending and requires acknowledgement.
  @param  Time                  The current alarm setting.

  @retval EFI_SUCCESS           The alarm settings were returned.
  @retval EFI_INVALID_PARAMETER Any parameter is NULL.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be retrieved due to a hardware error.
  @retval EFI_UNSUPPORTED       A wakeup timer is not supported on this platform.

**/
EFI_STATUS
EFIAPI
LibGetWakeupTime (
  OUT BOOLEAN     *Enabled,
  OUT BOOLEAN     *Pending,
  OUT EFI_TIME    *Time
  )
{
	return EFI_UNSUPPORTED;
}

/**
  Sets the system wakeup alarm clock time.

  @param  Enabled               Enable or disable the wakeup alarm.
  @param  Time                  If Enable is TRUE, the time to set the wakeup alarm for.

  @retval EFI_SUCCESS           If Enable is TRUE, then the wakeup alarm was enabled. If
                                Enable is FALSE, then the wakeup alarm was disabled.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be set due to a hardware error.
  @retval EFI_UNSUPPORTED       A wakeup timer is not supported on this platform.

**/
EFI_STATUS
EFIAPI
LibSetWakeupTime (
  IN BOOLEAN      Enabled,
  OUT EFI_TIME    *Time
  )
{
	return EFI_UNSUPPORTED;
}

/**
  This is the declaration of an EFI image entry point. This can be the entry point to an application
  written to this specification, an EFI boot service driver, or an EFI runtime driver.

  @param  ImageHandle           Handle that identifies the loaded image.
  @param  SystemTable           System Table for this image.

  @retval EFI_SUCCESS           The operation completed successfully.

**/
EFI_STATUS
EFIAPI
LibRtcInitialize (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{

	EFI_STATUS Status = EFI_SUCCESS;

	  /* Initialize library */
	  Pm8x41ImplLibInitialize();

	  /* Check current time offset */
	  UINT8 ponFlag = gPm8x41->pm8x41_reg_read(PM8916_PON_PBL_STATUS);

	  if (ponFlag && PM8916_PON_PBL_STATUS_XVDD_RB_OCCURRED)
	  {
		DEBUG((EFI_D_ERROR, "System reported power off. Resetting PMIC RTC time offset.\n"));
		mRtcOffset = DEFAULT_TIME_OFFSET;
	  }
	  else
	  {
		DEBUG((EFI_D_ERROR, "Read PMIC RTC offset from UEFI BS variable.\n"));
		mRtcOffset = DEFAULT_TIME_OFFSET;
	  }

	  /* Check if the RTC is on, else turn it on */
	  UINT8 controlReg = gPm8x41->pm8x41_reg_read(PM8916_RTC_CONTROL_ADDR);

	  if (!(controlReg & PM8xxx_RTC_ENABLE))
	  {
		DEBUG((EFI_D_ERROR, "Enabling PMIC RTC.\n"));
		controlReg |= PM8xxx_RTC_ENABLE;
		gPm8x41->pm8x41_reg_write( 
			PM8916_RTC_CONTROL_ADDR, 
			controlReg
		);
	  }

	  //
	  // Register for the virtual address change event
	  //
	  Status = gBS->CreateEventEx(
		  EVT_NOTIFY_SIGNAL,
		  TPL_NOTIFY,
		  LibRtcVirtualNotifyEvent,
		  NULL,
		  &gEfiEventVirtualAddressChangeGuid,
		  &mRtcVirtualAddrChangeEvent
	  );

	  if (EFI_ERROR(Status)) {
		  DEBUG((EFI_D_ERROR, "Failed to register event.\n"));
		  return Status;
	  }

	  mDeviceReady = TRUE;
	  return EFI_SUCCESS;
}
