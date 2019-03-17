#ifndef __BOOTAPP_H__
#define __BOOTAPP_H__

#include <IndustryStandard/SmBios.h>

#include <Guid/MdeModuleHii.h>
#include <Guid/StatusCodeDataTypeId.h>

#include <Protocol/Smbios.h>
#include <Protocol/HiiConfigAccess.h>

#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/PcdLib.h>
#include <Protocol/BootLogo.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/AbsolutePointer.h>
#include <LittleVgl/core/lvgl.h>
#include <LittleVgl/lv_conf.h>
#include <LittleVgl/core/lv_core/lv_refr.h>

#include <Resources/ReleaseStampStub.h>
#include <Resources/ReleaseInfo.h>

#endif
