#ifndef __LIBRARY_LKENV_H__
#define __LIBRARY_LKENV_H__

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/TimerLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/ArmLib.h>

typedef INT8 int8_t;
typedef INT16 int16_t;
typedef INT32 int32_t;
typedef INT64 int64_t;

typedef UINT8 uint8_t;
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
typedef UINT64 uint64_t;

typedef UINTN size_t;
typedef BOOLEAN bool;
typedef UINTN addr_t;
typedef UINTN paddr_t;

#define UINT_MAX MAX_UINTN

#define writel(v, a) MmioWrite32((UINTN)(a), (UINT32)(v))
#define readl(a) MmioRead32((UINTN)(a))
#define writeb(v, a) MmioWrite8((UINTN)(a), (UINT8)(v))
#define readb(a) MmioRead8((UINTN)(a))
#define writehw(v, a) MmioWrite16((UINTN)(a), (UINT16)(v))
#define readhw(a) MmioRead16((UINTN)(a))
#define RMWREG32(addr, startbit, width, val) writel((readl(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit)), addr)
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

#define true TRUE
#define false FALSE

#define strcmp(s1, s2) ((int)AsciiStrCmp((s1), (s2)))
#define strncmp(s1, s2, n) ((int)AsciiStrnCmp((s1), (s2), (n)))
#define strlen(s) ((size_t)AsciiStrLen((s)))
#define strlcpy(dst, src, n) AsciiStrCpyS((dst), (n), (src))
#define strstr(s1 , s2) AsciiStrStr((s1), (s2))
#define memset(s, c, n) SetMem((s), (UINTN)(n), (UINT8)(c))
#define memcpy(s1, s2, n) CopyMem((s1), (s2), (n))
#define memmove(s1, s2, n) CopyMem((s1), (s2), (n))
#define memcmp(s1, s2, n) ((int)CompareMem((s1), (s2), (n)))

#define va_list VA_LIST
#define offsetof(type, member) OFFSET_OF (type, member)
#define __PACKED __attribute__((packed))

#define ROUNDUP(a, b) (((a) + ((b)-1)) & ~((b)-1))
#define ROUNDDOWN(a, b) ((a) & ~((b)-1))
#define CACHE_LINE (ArmDataCacheLineLength())
#define IS_CACHE_LINE_ALIGNED(addr)  !((UINTN) (addr) & (CACHE_LINE - 1))

#define snprintf(s, n, fmt, ...) ((int)AsciiSPrint((s), (n), (fmt), ##__VA_ARGS__))

/* debug levels */
#define CRITICAL DEBUG_ERROR
#define ALWAYS DEBUG_ERROR
#define INFO DEBUG_INFO
#define SPEW DEBUG_VERBOSE

#if !defined(MDEPKG_NDEBUG)
#define dprintf(level, fmt, ...) do { \
                                   if (DebugPrintEnabled ()) { \
                                     CHAR8 __printbuf[100]; \
                                     UINTN __printindex; \
                                     CONST CHAR8 *__fmtptr = (fmt); \
                                     UINTN __fmtlen = AsciiStrSize(__fmtptr); \
                                     CopyMem(__printbuf, __fmtptr, __fmtlen); \
                                     __printbuf[__fmtlen-1] = 0; \
                                     for(__printindex=1; __printbuf[__printindex]; __printindex++) { \
                                       if (__printbuf[__printindex-1]=='%' && __printbuf[__printindex]=='s') \
                                         __printbuf[__printindex] = 'a'; \
                                     } \
                                     DEBUG(((level), __printbuf, ##__VA_ARGS__)); \
                                   } \
                                 } while(0)
#else
#define dprintf(level, fmt, ...)
#endif

#define ntohl(n) SwapBytes32(n)

#define dmb() ArmDataMemoryBarrier()
#define dsb() ArmDataSynchronizationBarrier()

#define mdelay(msecs) MicroSecondDelay ((msecs) * 1000)
#define udelay(usecs) MicroSecondDelay ((usecs))

#define arch_clean_invalidate_cache_range(start, len) WriteBackInvalidateDataCacheRange ((VOID *)(UINTN)(start), (UINTN)(len))
#define arch_invalidate_cache_range(start, len) InvalidateDataCacheRange ((VOID *)(UINTN)(start), (UINTN)(len));

#define __ALWAYS_INLINE __attribute__ ((always_inline))

#endif
