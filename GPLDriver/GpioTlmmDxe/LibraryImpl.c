#include <Base.h>

#include <Library/LKEnvLib.h>
#include <Library/QcomGpioTlmmLib.h>

#include "gpio_p.h"

QCOM_GPIO_TLMM_PROTOCOL *gGpioTlmm = NULL;

STATIC QCOM_GPIO_TLMM_PROTOCOL mInternalGpioTlmm = {
  msm_gpio_direction_input,
  msm_gpio_direction_output,
  msm_gpio_get_direction,
  msm_gpio_get,
  msm_gpio_set,

  msm_pinmux_set_function,
  msm_pinmux_set_drive_strength,
  msm_pinmux_set_pull,

  tlmm_set_hdrive_ctrl,
  tlmm_set_pull_ctrl,
};

#if defined(MDE_CPU_ARM)
#define BITS_PER_LONG 32
#elif defined(MDE_CPU_AARCH64)
#define BITS_PER_LONG 64
#else
#error Unknown architecture
#endif

#define GENMASK(h, l) \
	(((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

STATIC inline int fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

STATIC inline unsigned long __fls(unsigned long word)
{
	int num = BITS_PER_LONG - 1;

#if BITS_PER_LONG == 64
	if (!(word & (~0ul << 32))) {
		num -= 32;
		word <<= 32;
	}
#endif
	if (!(word & (~0ul << (BITS_PER_LONG-16)))) {
		num -= 16;
		word <<= 16;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-8)))) {
		num -= 8;
		word <<= 8;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-4)))) {
		num -= 4;
		word <<= 4;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-2)))) {
		num -= 2;
		word <<= 2;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-1))))
		num -= 1;
	return num;
}

#if BITS_PER_LONG == 32
STATIC inline int fls64(UINT64 x)
{
	UINT32 h = x >> 32;
	if (h)
		return fls(h) + 32;
	return fls(x);
}
#elif BITS_PER_LONG == 64
STATIC inline int fls64(UINT64 x)
{
	if (x == 0)
		return 0;
	return __fls(x) + 1;
}
#else
#error BITS_PER_LONG not 32 or 64
#endif

STATIC inline __attribute__((const))
int __ilog2_u32(UINT32 n)
{
	return fls(n) - 1;
}

STATIC inline __attribute__((const))
int __ilog2_u64(UINT64 n)
{
	return fls64(n) - 1;
}

#define ilog2(n)				\
(						\
	__builtin_constant_p(n) ? (		\
		(n) < 2 ? 0 :			\
		(n) & (1ULL << 63) ? 63 :	\
		(n) & (1ULL << 62) ? 62 :	\
		(n) & (1ULL << 61) ? 61 :	\
		(n) & (1ULL << 60) ? 60 :	\
		(n) & (1ULL << 59) ? 59 :	\
		(n) & (1ULL << 58) ? 58 :	\
		(n) & (1ULL << 57) ? 57 :	\
		(n) & (1ULL << 56) ? 56 :	\
		(n) & (1ULL << 55) ? 55 :	\
		(n) & (1ULL << 54) ? 54 :	\
		(n) & (1ULL << 53) ? 53 :	\
		(n) & (1ULL << 52) ? 52 :	\
		(n) & (1ULL << 51) ? 51 :	\
		(n) & (1ULL << 50) ? 50 :	\
		(n) & (1ULL << 49) ? 49 :	\
		(n) & (1ULL << 48) ? 48 :	\
		(n) & (1ULL << 47) ? 47 :	\
		(n) & (1ULL << 46) ? 46 :	\
		(n) & (1ULL << 45) ? 45 :	\
		(n) & (1ULL << 44) ? 44 :	\
		(n) & (1ULL << 43) ? 43 :	\
		(n) & (1ULL << 42) ? 42 :	\
		(n) & (1ULL << 41) ? 41 :	\
		(n) & (1ULL << 40) ? 40 :	\
		(n) & (1ULL << 39) ? 39 :	\
		(n) & (1ULL << 38) ? 38 :	\
		(n) & (1ULL << 37) ? 37 :	\
		(n) & (1ULL << 36) ? 36 :	\
		(n) & (1ULL << 35) ? 35 :	\
		(n) & (1ULL << 34) ? 34 :	\
		(n) & (1ULL << 33) ? 33 :	\
		(n) & (1ULL << 32) ? 32 :	\
		(n) & (1ULL << 31) ? 31 :	\
		(n) & (1ULL << 30) ? 30 :	\
		(n) & (1ULL << 29) ? 29 :	\
		(n) & (1ULL << 28) ? 28 :	\
		(n) & (1ULL << 27) ? 27 :	\
		(n) & (1ULL << 26) ? 26 :	\
		(n) & (1ULL << 25) ? 25 :	\
		(n) & (1ULL << 24) ? 24 :	\
		(n) & (1ULL << 23) ? 23 :	\
		(n) & (1ULL << 22) ? 22 :	\
		(n) & (1ULL << 21) ? 21 :	\
		(n) & (1ULL << 20) ? 20 :	\
		(n) & (1ULL << 19) ? 19 :	\
		(n) & (1ULL << 18) ? 18 :	\
		(n) & (1ULL << 17) ? 17 :	\
		(n) & (1ULL << 16) ? 16 :	\
		(n) & (1ULL << 15) ? 15 :	\
		(n) & (1ULL << 14) ? 14 :	\
		(n) & (1ULL << 13) ? 13 :	\
		(n) & (1ULL << 12) ? 12 :	\
		(n) & (1ULL << 11) ? 11 :	\
		(n) & (1ULL << 10) ? 10 :	\
		(n) & (1ULL <<  9) ?  9 :	\
		(n) & (1ULL <<  8) ?  8 :	\
		(n) & (1ULL <<  7) ?  7 :	\
		(n) & (1ULL <<  6) ?  6 :	\
		(n) & (1ULL <<  5) ?  5 :	\
		(n) & (1ULL <<  4) ?  4 :	\
		(n) & (1ULL <<  3) ?  3 :	\
		(n) & (1ULL <<  2) ?  2 :	\
		1 ) :				\
	(sizeof(n) <= 4) ?			\
	__ilog2_u32(n) :			\
	__ilog2_u64(n)				\
 )

STATIC inline __attribute__((const))
int __order_base_2(unsigned long n)
{
	return n > 1 ? ilog2(n - 1) + 1 : 0;
}

#define order_base_2(n)				\
(						\
	__builtin_constant_p(n) ? (		\
		((n) == 0 || (n) == 1) ? 0 :	\
		ilog2((n) - 1) + 1) :		\
	__order_base_2(n)			\
)

STATIC inline UINTN gpio_tlmm_platform_ctl_reg(UINTN id) {
  return PcdGet64 (PcdGpioTlmmCtlOffset) + PcdGet64 (PcdGpioTlmmCtlElementSize) * id;
}

STATIC inline UINTN gpio_tlmm_platform_io_reg(UINTN id) {
  return PcdGet64 (PcdGpioTlmmIoOffset) + PcdGet64 (PcdGpioTlmmIoElementSize) * id;
}

EFI_STATUS msm_gpio_direction_input(UINTN id)
{
  UINT32 val;

  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_ctl_reg(id));
  val &= ~BIT(PcdGet64 (PcdGpioTlmmOeBit));
  writel(val, PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_ctl_reg(id));

  return EFI_SUCCESS;
}

EFI_STATUS msm_gpio_direction_output(UINTN id, UINTN value)
{
  UINT32 val;

  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_io_reg(id));
  if (value)
    val |= BIT(PcdGet64 (PcdGpioTlmmOutBit));
  else
    val &= ~BIT(PcdGet64 (PcdGpioTlmmOutBit));
  writel(val, PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_io_reg(id));

  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_ctl_reg(id));
  val |= BIT(PcdGet64 (PcdGpioTlmmOeBit));
  writel(val, PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_ctl_reg(id));

  return EFI_SUCCESS;
}

GPIO_DIRECTION msm_gpio_get_direction(UINTN id)
{
  UINT32 val;

  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_ctl_reg(id));

  return val & BIT(PcdGet64 (PcdGpioTlmmOeBit)) ? GPIO_DIRECTION_OUT : GPIO_DIRECTION_IN;
}

UINTN msm_gpio_get(UINTN id)
{
  UINT32 val;

  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_io_reg(id));
  return !!(val & BIT(PcdGet64 (PcdGpioTlmmInBit)));
}

VOID msm_gpio_set(UINTN id, UINTN value)
{
  UINT32 val;

  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_io_reg(id));
  if (value)
    val |= BIT(PcdGet64 (PcdGpioTlmmOutBit));
  else
    val &= ~BIT(PcdGet64 (PcdGpioTlmmOutBit));
  writel(val, PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_io_reg(id));
}

EFI_STATUS msm_pinmux_set_function(UINTN id, UINTN function)
{
  UINT32 val, mask;

  mask = GENMASK(PcdGet64 (PcdGpioTlmmMuxBit) + order_base_2(PcdGet64 (PcdGpioTlmmNumFunctions)) - 1, PcdGet64 (PcdGpioTlmmMuxBit));

  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_ctl_reg(id));
  val &= ~mask;
  val |= function << PcdGet64 (PcdGpioTlmmMuxBit);
  writel(val, PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_ctl_reg(id));

  return EFI_SUCCESS;
}

EFI_STATUS msm_pinmux_set_drive_strength(UINTN id, UINTN ma)
{
  UINT32 val;
  UINTN mask, bit;
  UINT32 arg;

  /* Check for invalid values */
  if (ma > 16 || ma < 2 || (ma % 2) != 0)
    return EFI_INVALID_PARAMETER;

  arg = (ma / 2) - 1;
  mask = 7;
  bit = PcdGet64 (PcdGpioTlmmDrvBit);

  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_ctl_reg(id));
  val &= ~(mask << bit);
  val |= arg << bit;
  writel(val, PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_ctl_reg(id));

  return EFI_SUCCESS;
}

EFI_STATUS msm_pinmux_set_pull(UINTN id, GPIO_PULL pull)
{
  UINT32 val;
  UINTN mask, bit;

  mask = 3;
  bit = PcdGet64 (PcdGpioTlmmPullBit);

  if (pull & ~mask) {
    return EFI_INVALID_PARAMETER;
  }

  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_ctl_reg(id));
  val &= ~(mask << bit);
  val |= pull << bit;
  writel(val, PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_ctl_reg(id));

  return EFI_SUCCESS;
}

RETURN_STATUS
EFIAPI
GpioTlmmImplLibInitialize (
  VOID
  )
{
  gGpioTlmm = &mInternalGpioTlmm;

  return RETURN_SUCCESS;
}
