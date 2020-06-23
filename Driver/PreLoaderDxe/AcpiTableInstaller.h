#ifndef _ACPITABLEINSTALLER_PRIVATE_H
#define _ACPITABLEINSTALLER_PRIVATE_H

#include <PiDxe.h>
#include <Uefi.h>

#include <Configuration/Hob.h>
#include <Pi/PiFirmwareFile.h>

#include <Library/BaseLib.h>
#include <Library/BmpSupportLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/EfiResetSystemLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/Bmp.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/GraphicsOutput.h>

#include <Library/BgraRgbaConvert.h>
#include <Library/lodepng.h>
#include <LittleVgl/core/lv_core/lv_refr.h>
#include <LittleVgl/core/lvgl.h>
#include <LittleVgl/lv_conf.h>

EFI_STATUS
EFIAPI
InstallAcpiTables();

#endif // _ACPITABLEINSTALLER_PRIVATE_H