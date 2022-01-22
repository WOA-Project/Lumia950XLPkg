#ifndef _DEVICE_MEMORY_MAP_H_
#define _DEVICE_MEMORY_MAP_H_

#include <Library/ArmLib.h>

#define MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT 64

/* Below flag is used for system memory */
#define SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES                               \
  EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED |        \
      EFI_RESOURCE_ATTRIBUTE_TESTED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |     \
      EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |                               \
      EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |                         \
      EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |                            \
      EFI_RESOURCE_ATTRIBUTE_EXECUTION_PROTECTABLE

typedef enum { NoHob, AddMem, AddDev, MaxMem } DeviceMemoryAddHob;

typedef struct {
  EFI_PHYSICAL_ADDRESS         Address;
  UINT64                       Length;
  EFI_RESOURCE_TYPE            ResourceType;
  EFI_RESOURCE_ATTRIBUTE_TYPE  ResourceAttribute;
  ARM_MEMORY_REGION_ATTRIBUTES ArmAttributes;
  DeviceMemoryAddHob           HobOption;
  EFI_MEMORY_TYPE              MemoryType;
} ARM_MEMORY_REGION_DESCRIPTOR_EX, *PARM_MEMORY_REGION_DESCRIPTOR_EX;

static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx[] = {
    /* Address,	  Length,     ResourceType, Resource Attribute, ARM MMU
       Attribute,                  HobOption, EFI Memory Type */

    /* DDR Regions */
    /* DBI Dump */
    {0x00010000, 0x00014000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     NoHob, EfiConventionalMemory},
    /* DDR Health Mon */
    {0x00024000, 0x00002000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     NoHob, EfiReservedMemoryType},
    /* HLOS 0 */
    {0x00100000, 0x00100000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiConventionalMemory},
    /* UEFI FD */
    {0x00200000, 0x00100000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiBootServicesCode},
    /* MPPark Code */
    {0x00300000, 0x00080000, EFI_RESOURCE_MEMORY_RESERVED,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE,
     ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED, AddMem,
     EfiRuntimeServicesCode},
    /* FBPT Payload */
    {0x00380000, 0x00001000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED, AddMem,
     EfiRuntimeServicesData},
    /* DBG2 */
    {0x00381000, 0x00004000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED, AddMem, EfiLoaderData},
    /* Capsule Header */
    {0x00385000, 0x00001000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED, AddMem,
     EfiRuntimeServicesData},
    /* TPM Control Area */
    {0x00386000, 0x00003000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED, AddMem,
     EfiRuntimeServicesData},
    /* UEFI Info Block */
    {0x00389000, 0x00001000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED, AddMem,
     EfiRuntimeServicesData},
    /* Reset Data */
    {0x0038A000, 0x00004000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED, AddMem,
     EfiRuntimeServicesData},
    /* Reser. Uncached0 */
    {0x0038E000, 0x00072000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED, AddMem,
     EfiBootServicesData},
    /* Display Reserved */
    {0x00400000, 0x00800000, EFI_RESOURCE_MEMORY_RESERVED,
     EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH, AddMem, EfiMaxMemoryType},
    /* UEFI Stack */
    {0x00C00000, 0x00040000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiBootServicesData},
    /* CPU Vectors */
    {0x00C40000, 0x00010000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiBootServicesCode},
    /* Reser. Cached 0 */
    {0x00C50000, 0x000B0000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiBootServicesData},
    /* HLOS 1 */
    {0x00D00000, 0x03300000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiBootServicesData},
    /* HLOS 2 */
    {0x04000000, 0x02500000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiConventionalMemory},
    /* TZ Apps */
    {0x06500000, 0x00500000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddMem, EfiReservedMemoryType},
    /* SMEM */
    {0x06A00000, 0x00200000, EFI_RESOURCE_MEMORY_RESERVED,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE,
     ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED, AddMem,
     EfiReservedMemoryType},
    /* Hypervisor */
    {0x06C00000, 0x00100000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddMem, EfiReservedMemoryType},
    /* TZ */
    {0x06D00000, 0x00160000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddMem, EfiReservedMemoryType},
    /* ADSP_EFS */
    {0x06E60000, 0x00020000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddMem, EfiReservedMemoryType},
    /* MPSS_EFS / SBL */
    {0x06E80000, 0x00180000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddMem, EfiReservedMemoryType},
    /* Subsy Res. 1/DHMS */
    {0x07000000, 0x07F00000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddMem, EfiReservedMemoryType},
    /* CNSS_DEBUG */
    {0x0EF00000, 0x00300000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddMem, EfiReservedMemoryType},
    /* HLOS 3 */
    {0x0F200000, 0x10E00000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiConventionalMemory},
    /* HLOS 4 Part 1 */
    {0x20000000, 0x40000000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiConventionalMemory},
    /* HLOS 4, Part 2.1 */
    {0x80000000, 0x30000000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiConventionalMemory},
    /* PreLoader information block */
    {0xb0000000, 0x00001000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiBootServicesData},
    /* HLOS 4, Part 2.2 */
    {0xb0001000, 0x2ffff000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiConventionalMemory},
#if MEMORY_4GB == 1
    /* HLOS 4, Part 2.3 */
    {0xe0000000, 0x18000000, EFI_RESOURCE_SYSTEM_MEMORY,
     SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiConventionalMemory},
#endif

    /* Other memory regions */
    /* IMEM SMEM Base */
    {0xFE805000, 0x00001000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     NoHob, EfiConventionalMemory},
    /* IMEM Cookie Base */
    {0xFE87F000, 0x00001000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiConventionalMemory},
    /* MSM8992 platform needs another section of IMEM */
#if SILICON_PLATFORM == 8992
    {0xFE80F000, 0x00001000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiConventionalMemory},
#endif

    /* Register regions */
    /* TERMINATOR */
    {0xF9000000, 0x00113000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* GCC CLK CTL */
    {0xFC400000, 0x00002000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* RPM MSG RAM */
    {0xFC428000, 0x00008000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* MMSS */
    {0xFD800000, 0x001DC000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* MPM2 MPM */
    {0xFC4A0000, 0x0000C000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* PMIC ARB SPMI */
    {0xFC4C0000, 0x00010000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* CRYPTO0 CRYPTO */
    {0xFD404000, 0x0001C000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* CRYPTO1 CRYPTO */
    {0xFD444000, 0x0001C000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* CRYPTO2 CRYPTO */
    {0xFD3C4000, 0x0001C000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* Security Ctrl */
    {0xFC4B8000, 0x00007000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* SS SDC1/2/3/4 */
    {0xF9800000, 0x000E7000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* SS BLSP1/2 */
    {0xF9900000, 0x00069000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* SS USBOTG */
    {0xF9A40000, 0x00016000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* SS USB3PHY */
    {0xF9B38000, 0x00008000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* USB30 PRIM */
    {0xF9200000, 0x0010D000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* PERIPH_SS_PRNG */
    {0xF9BFF000, 0x00001000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* TLMM CSR */
    {0xFD510000, 0x00004000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* TCSR TCSR MUTEX */
    {0xFD484000, 0x00002000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* TCSR TCSR REGS */
    {0xFD4A0000, 0x00010000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* PCIE WRAPPER AXI */
    {0xF8800000, 0x00800000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* PCIE WRAPPER AHB */
    {0xFC528000, 0x00008000, EFI_RESOURCE_MEMORY_MAPPED_IO,
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
     AddDev, EfiMemoryMappedIO},
    /* Terminator */
    {}};

#endif
