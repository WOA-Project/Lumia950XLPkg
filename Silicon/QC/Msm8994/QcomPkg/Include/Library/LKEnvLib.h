#ifndef __LIBRARY_LKENV_H__
#define __LIBRARY_LKENV_H__

#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/TimerLib.h>

#include "minstdbool.h"
#include "minstdint.h"


#define REG32(addr) ((volatile uint32_t *)(addr))
#define writel_rt(v, a) (*REG32(a) = (v))
#define readl_rt(a) (*REG32(a))

#define writel(v, a) MmioWrite32((UINTN)(a), (UINT32)(v))
#define readl(a) MmioRead32((UINTN)(a))
#define writeb(v, a) MmioWrite8((UINTN)(a), (UINT8)(v))
#define readb(a) MmioRead8((UINTN)(a))
#define writehw(v, a) MmioWrite16((UINTN)(a), (UINT16)(v))
#define readhw(a) MmioRead16((UINTN)(a))
#define RMWREG32(addr, startbit, width, val)                                   \
  writel(                                                                      \
      (readl(addr) & ~(((1 << (width)) - 1) << (startbit))) |                  \
          ((val) << (startbit)),                                               \
      addr)
#define BIT(bit) (1 << (bit))

#ifdef MSM_SECURE_IO
#define readl_relaxed secure_readl
#define writel_relaxed secure_writel
#else
#define readl_relaxed readl
#define writel_relaxed writel
#endif

#define NO_ERROR 0
#define ERROR -1
#define ERR_NOT_FOUND -2
#define ERR_NO_MEMORY -5
#define ERR_NOT_VALID -7
#define ERR_INVALID_ARGS -8
#define ERR_IO -20
#define ERR_NOT_SUPPORTED -24

#include "minstdbool.h"
#include "minstring.h"

#define va_list VA_LIST
#define offsetof(type, member) OFFSET_OF(type, member)
#define __PACKED __attribute__((packed))

#define ROUNDUP(a, b) (((a) + ((b)-1)) & ~((b)-1))
#define ROUNDDOWN(a, b) ((a) & ~((b)-1))
#define CACHE_LINE (ArmDataCacheLineLength())
#define IS_CACHE_LINE_ALIGNED(addr) !((UINTN)(addr) & (CACHE_LINE - 1))

#define snprintf(s, n, fmt, ...)                                               \
  ((int)AsciiSPrint((s), (n), (fmt), ##__VA_ARGS__))

/* debug levels */
#define CRITICAL DEBUG_ERROR
#define ALWAYS DEBUG_ERROR
#define INFO DEBUG_INFO
#define SPEW DEBUG_VERBOSE

#if !defined(MDEPKG_NDEBUG)
#define dprintf(level, fmt, ...)                                               \
  do {                                                                         \
    if (DebugPrintEnabled()) {                                                 \
      CHAR8 __printbuf[100];                                                   \
      UINTN __printindex;                                                      \
      CONST CHAR8 *__fmtptr = (fmt);                                           \
      UINTN        __fmtlen = AsciiStrSize(__fmtptr);                          \
      CopyMem(__printbuf, __fmtptr, __fmtlen);                                 \
      __printbuf[__fmtlen - 1] = 0;                                            \
      for (__printindex = 1; __printbuf[__printindex]; __printindex++) {       \
        if (__printbuf[__printindex - 1] == '%' &&                             \
            __printbuf[__printindex] == 's')                                   \
          __printbuf[__printindex] = 'a';                                      \
      }                                                                        \
      DEBUG(((level), __printbuf, ##__VA_ARGS__));                             \
    }                                                                          \
  } while (0)
#else
#define dprintf(level, fmt, ...)
#endif

#define ntohl(n) SwapBytes32(n)

#define dmb() ArmDataMemoryBarrier()
#define dsb() ArmDataSynchronizationBarrier()

#define mdelay(msecs) MicroSecondDelay((msecs)*1000)
#define udelay(usecs) MicroSecondDelay((usecs))

#define arch_clean_invalidate_cache_range(start, len)                          \
  WriteBackInvalidateDataCacheRange((VOID *)(UINTN)(start), (UINTN)(len))
#define arch_invalidate_cache_range(start, len)                                \
  InvalidateDataCacheRange((VOID *)(UINTN)(start), (UINTN)(len));

#define __ALWAYS_INLINE __attribute__((always_inline))

#define ROUND_TO_PAGE(x) (x & (~(EFI_PAGE_SIZE - 1)))

#endif
