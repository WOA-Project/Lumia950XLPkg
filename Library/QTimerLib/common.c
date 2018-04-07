#include <Base.h>

UINT32 qtimer_get_frequency(VOID)
{
  UINT32 freq;

  /* Read the Global counter frequency */
  /* freq = readl(QTMR_V1_CNTFRQ); */
  /* TODO: remove this when bootchaint sets up the frequency. */
  freq = 19200000;

  return freq;
}
