#include <Library/BaseLib.h>
#include <Library/PlatformMemoryMapLib.h>

static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx[] = {
    /* Name               Address     Length      HobOption        ResourceAttribute    ArmAttributes
                                                          ResourceType          MemoryType */

    /* DDR Regions */
    {"DBI Dump",          0x00010000, 0x00014000, NoHob,  MMAP_IO, INITIALIZED,  Conv,   NS_DEVICE},
    {"DDR Health Mon",    0x00024000, 0x00002000, NoHob,  MMAP_IO, INITIALIZED,  Reserv, NS_DEVICE},
    {"HLOS 0",            0x00100000, 0x00100000, AddMem, SYS_MEM, SYS_MEM_CAP,  Conv,   WRITE_BACK},
    {"UEFI FD",           0x00200000, 0x00100000, AddMem, SYS_MEM, SYS_MEM_CAP,  BsCode, WRITE_BACK},
    {"MPPark Code",       0x00300000, 0x00080000, AddMem, MEM_RES, UNCACHEABLE,  RtCode, UNCACHED_UNBUFFERED},
    {"FBPT Payload",      0x00380000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP,  RtData, UNCACHED_UNBUFFERED},
    {"DBG2",              0x00381000, 0x00004000, AddMem, SYS_MEM, SYS_MEM_CAP,  RtData, UNCACHED_UNBUFFERED},
    {"Capsule Header",    0x00385000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP,  RtData, UNCACHED_UNBUFFERED},
    {"TPM Control Area",  0x00386000, 0x00003000, AddMem, SYS_MEM, SYS_MEM_CAP,  RtData, UNCACHED_UNBUFFERED},
    {"UEFI Info Block",   0x00389000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP,  RtData, UNCACHED_UNBUFFERED},
    {"Reset Data",        0x0038A000, 0x00004000, AddMem, SYS_MEM, SYS_MEM_CAP,  RtData, UNCACHED_UNBUFFERED},
    {"Reser. Uncached0",  0x0038E000, 0x00002000, AddMem, SYS_MEM, SYS_MEM_CAP,  RtData, UNCACHED_UNBUFFERED}, /* It's tricky tricky tricky */
    {"Reser. Uncached0",  0x00390000, 0x00070000, AddMem, SYS_MEM, SYS_MEM_CAP,  BsData, UNCACHED_UNBUFFERED}, /* There goes the other part */
    {"Display Reserved",  0x00400000, 0x00800000, AddMem, MEM_RES, WRITE_THROUGH, MaxMem, WRITE_THROUGH},
    {"UEFI Stack",        0x00C00000, 0x00040000, AddMem, SYS_MEM, SYS_MEM_CAP,  BsData, WRITE_BACK},
    {"CPU Vectors",       0x00C40000, 0x00010000, AddMem, SYS_MEM, SYS_MEM_CAP,  BsCode, WRITE_BACK},
    {"Reser. Cached 0",   0x00C50000, 0x000B0000, AddMem, SYS_MEM, SYS_MEM_CAP,  BsData, WRITE_BACK},
    {"HLOS 1",            0x00D00000, 0x03300000, AddMem, SYS_MEM, SYS_MEM_CAP,  BsData, WRITE_BACK},
    {"HLOS 2",            0x04000000, 0x02500000, AddMem, SYS_MEM, SYS_MEM_CAP,  Conv,   WRITE_BACK},
    {"TZ Apps",           0x06500000, 0x00500000, AddMem, SYS_MEM, SYS_MEM_CAP,  Reserv, NS_DEVICE},
    {"SMEM",              0x06A00000, 0x00200000, AddMem, MEM_RES, UNCACHEABLE,  Reserv, UNCACHED_UNBUFFERED},
    {"Hypervisor",        0x06C00000, 0x00100000, AddMem, SYS_MEM, SYS_MEM_CAP,  Reserv, NS_DEVICE},
#if SILICON_PLATFORM == 8992
    {"TZ",                0x06D00000, 0x00200000, AddMem, SYS_MEM, SYS_MEM_CAP,  Reserv, NS_DEVICE},
    {"MPSS_EFS / SBL",    0x06F00000, 0x00180000, AddMem, SYS_MEM, SYS_MEM_CAP,  Reserv, NS_DEVICE},
    {"ADSP_EFS",          0x07080000, 0x00020000, AddMem, SYS_MEM, SYS_MEM_CAP,  Reserv, NS_DEVICE},
    {"HLOS 3",            0x070A0000, 0x00360000, AddMem, SYS_MEM, SYS_MEM_CAP,  Conv,   WRITE_BACK},
    {"Subsys Reser. 1",   0x07400000, 0x07B00000, AddMem, SYS_MEM, SYS_MEM_CAP,  Reserv, NS_DEVICE},
    {"CNSS_DEBUG",        0x0EF00000, 0x00300000, AddMem, SYS_MEM, SYS_MEM_CAP,  Reserv, NS_DEVICE},
    {"HLOS 4",            0x0F200000, 0x10E00000, AddMem, SYS_MEM, SYS_MEM_CAP,  Conv,   WRITE_BACK},
#else
    {"TZ",                0x06D00000, 0x00160000, AddMem, SYS_MEM, SYS_MEM_CAP,  Reserv, NS_DEVICE},
    {"ADSP_EFS",          0x06E60000, 0x00020000, AddMem, SYS_MEM, SYS_MEM_CAP,  Reserv, NS_DEVICE},
    {"MPSS_EFS / SBL",    0x06E80000, 0x00180000, AddMem, SYS_MEM, SYS_MEM_CAP,  Reserv, NS_DEVICE},
    {"Subsy Res. 1/DHMS", 0x07000000, 0x07F00000, AddMem, SYS_MEM, SYS_MEM_CAP,  Reserv, NS_DEVICE},
    {"CNSS_DEBUG",        0x0EF00000, 0x00300000, AddMem, SYS_MEM, SYS_MEM_CAP,  Reserv, NS_DEVICE},
    {"HLOS 3",            0x0F200000, 0x10E00000, AddMem, SYS_MEM, SYS_MEM_CAP,  Conv,   WRITE_BACK},
#endif

    /* RAM partition regions */
    {"RAM Partition",     0x20000000, 0x40000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"RAM Partition",     0x80000000, 0x30000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"Preloader Block",   0xB0000000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"RAM Partition",     0xB0001000, 0x2FFFF000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
#if MEMORY_4GB == 1
    {"RAM Partition",     0xE0000000, 0x18000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
#endif

    /* Other memory regions */
    {"IMEM SMEM Base",    0xFE805000, 0x00001000, NoHob,  MMAP_IO, INITIALIZED,  Conv,   NS_DEVICE},
#if SILICON_PLATFORM == 8992
    {"IMEM Cookie Base",  0xFE80F000, 0x00001000, AddDev, MMAP_IO, INITIALIZED,  Conv,   NS_DEVICE},
#else
    {"IMEM Cookie Base",  0xFE87F000, 0x00001000, AddDev, MMAP_IO, INITIALIZED,  Conv,   NS_DEVICE},
#endif

    /* Register regions */
    {"TERMINATOR",        0xF9000000, 0x00113000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"GCC CLK CTL",       0xFC400000, 0x00002000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"RPM MSG RAM",       0xFC428000, 0x00008000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"MMSS",              0xFD800000, 0x001DC000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"MPM2 MPM",          0xFC4A0000, 0x0000C000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"PMIC ARB SPMI",     0xFC4C0000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"CRYPTO0 CRYPTO",    0xFD404000, 0x0001C000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"CRYPTO1 CRYPTO",    0xFD444000, 0x0001C000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"CRYPTO2 CRYPTO",    0xFD3C4000, 0x0001C000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"Security Ctrl",     0xFC4B8000, 0x00007000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"SS SDC1/2/3/4",     0xF9800000, 0x000E7000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"SS BLSP1/2",        0xF9900000, 0x00069000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"SS USBOTG",         0xF9A40000, 0x00016000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"SS USB3PHY",        0xF9B38000, 0x00008000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"USB30 PRIM",        0xF9200000, 0x0010D000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"PERIPH_SS_PRNG",    0xF9BFF000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"TLMM CSR",          0xFD510000, 0x00004000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"TCSR TCSR MUTEX",   0xFD484000, 0x00002000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"TCSR TCSR REGS",    0xFD4A0000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
#if SILICON_PLATFORM == 8992
    {"PCIE WRAPPER AXI",  0xFF000000, 0x00800000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"PCIE WRAPPER AHB",  0xFC520000, 0x00008000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
#else
    {"PCIE WRAPPER AXI",  0xF8800000, 0x00800000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
    {"PCIE WRAPPER AHB",  0xFC528000, 0x00008000, AddDev, MMAP_IO, UNCACHEABLE,  MmIO,   NS_DEVICE},
#endif

    /* Terminator for MMU */
    {"Terminator", 0, 0, 0, 0, 0, 0, 0}};

ARM_MEMORY_REGION_DESCRIPTOR_EX *GetPlatformMemoryMap()
{
  return gDeviceMemoryDescriptorEx;
}