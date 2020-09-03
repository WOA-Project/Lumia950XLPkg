#ifndef __ACPI_LUMIA950XL_PLATFORM_H__
#define __ACPI_LUMIA950XL_PLATFORM_H__

#include <IndustryStandard/Acpi.h>
#include <Library/AcpiLib.h>

#define SIGNATURE4(a, b, c, d) (((a) | (b << 8)) | ((((c) | (d << 8)) << 16)))

#define SIGNATURE8(a, b, c, d, e, f, g, h)                                     \
  (SIGNATURE4(a, b, c, d) | ((UINT64)(SIGNATURE4(e, f, g, h)) << 32))

#define ACPI_OEM_ID 'Q', 'C', 'O', 'M', ' ', ' '
#define ACPI_OEM_TABLE_ID SIGNATURE8('Q', 'C', 'O', 'M', 'E', 'D', 'K', '2')
#define ACPI_OEM_REVISION 0x00008994
#define ACPI_CREATOR_ID SIGNATURE4('Q', 'C', 'O', 'M')
#define ACPI_CREATOR_REVISION 0x00000002

#define ACPI_VENDOR_ID SIGNATURE4('Q', 'C', 'O', 'M')

#define SCSS_SYS_POR 0x01
#define HWIO_SCSS_RESET_ADDR 0x009020B4

#define AP_CPU_COUNT 8

#endif