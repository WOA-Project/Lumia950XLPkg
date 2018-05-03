#ifndef _RAMDISK_DEVICE_PATH_INFO_
#define _RAMDISK_DEVICE_PATH_INFO_

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>

#define PLAT_RAMDISK_DEVICE_GUID \
    { 0x23333cc1, 0xd4d0, 0x4B11, { 0xAB, 0x43, 0x8A, 0xB2, 0x3D, 0xD0, 0xE2, 0xB0} }

typedef struct {
	VENDOR_DEVICE_PATH  Ramdisk;
	EFI_DEVICE_PATH     End;
} RAMDISK_DEVICE_PATH;

RAMDISK_DEVICE_PATH mRamdiskDevicePath =
{
	{
		{
			HARDWARE_DEVICE_PATH,
			HW_VENDOR_DP,
			{ sizeof(VENDOR_DEVICE_PATH), 0 }
		},
		PLAT_RAMDISK_DEVICE_GUID
	},
	{
		END_DEVICE_PATH_TYPE,
		END_ENTIRE_DEVICE_PATH_SUBTYPE,
		{ sizeof(EFI_DEVICE_PATH_PROTOCOL), 0 }
	}
};

#endif