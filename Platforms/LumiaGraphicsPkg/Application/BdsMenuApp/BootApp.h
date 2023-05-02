#ifndef __BOOTAPP_H__
#define __BOOTAPP_H__

#include <IndustryStandard/SmBios.h>

#include <Guid/MdeModuleHii.h>
#include <Guid/StatusCodeDataTypeId.h>

#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>

#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/AbsolutePointer.h>
#include <Protocol/BootLogo.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/Smbios.h>

#include <LittleVgl/core/lv_core/lv_refr.h>
#include <LittleVgl/core/lvgl.h>
#include <LittleVgl/lv_conf.h>

#include <Resources/ReleaseStampStub.h>
// Must comes in order
#include <Resources/ReleaseInfo.h>

#endif
