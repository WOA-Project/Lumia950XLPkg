#include <Base.h>
#include <Library/LKEnvLib.h>
#include <Library/QcomDxeTimerLib.h>

#include "qtimer_p.h"
#include "qtimer_mmap_hw.h"

#define QTMR_TIMER_CTRL_ENABLE          (1 << 0)
#define QTMR_TIMER_CTRL_INT_MASK        (1 << 1)

STATIC UINT64 mTimerTicks;

VOID
EFIAPI
LibQcomDxeTimerEnable (
  UINT64 TimerTicks
  )
{
  UINT32 ctrl;

  /* Set Physical Down Counter */
  writel(TimerTicks, QTMR_V1_CNTP_TVAL);
  dsb();

  ctrl = readl(QTMR_V1_CNTP_CTL);

  /* Program CTRL Register */
  ctrl |= QTMR_TIMER_CTRL_ENABLE;
  ctrl &= ~QTMR_TIMER_CTRL_INT_MASK;

  writel(ctrl, QTMR_V1_CNTP_CTL);
  dsb();

  mTimerTicks = TimerTicks;
}

VOID
EFIAPI
LibQcomDxeTimerDisable (
  VOID
  )
{
  UINT32 ctrl;

  ctrl = readl(QTMR_V1_CNTP_CTL);

  /* program cntrl register */
  ctrl &= ~QTMR_TIMER_CTRL_ENABLE;
  ctrl |= QTMR_TIMER_CTRL_INT_MASK;

  writel(ctrl, QTMR_V1_CNTP_CTL);
  dsb();
}

UINTN
EFIAPI
LibQcomDxeTimerGetFreq (
  VOID
  )
{
  return qtimer_get_frequency();
}

VOID
EFIAPI
LibQcomDxeTimerFinishIrq (
  VOID
  )
{
  // Program the down counter again to get an interrupt after timer_interval msecs
  writel(mTimerTicks, QTMR_V1_CNTP_TVAL);
  dsb();
}
