#include <PiDxe.h>
#include <Library/LKEnvLib.h>
#include <Library/QcomClockLib.h>
#include <Library/QcomGpioTlmmLib.h>

/* Configure UART clock based on the UART block id*/
VOID LibQcomPlatformUartDmClockConfig(UINT8 id)
{
  int ret;
  CHAR8 iclk[64];
  CHAR8 cclk[64];

  snprintf(iclk, sizeof(iclk), "uart%u_iface_clk", id);
  snprintf(cclk, sizeof(cclk), "uart%u_core_clk", id);

  ret = gClock->clk_get_set_enable(iclk, 0, 1);
  if(ret)
  {
    dprintf(CRITICAL, "failed to set uart%u_iface_clk ret = %d\n", id, ret);
    ASSERT(0);
  }

  ret = gClock->clk_get_set_enable(cclk, 7372800, 1);
  if(ret)
  {
    dprintf(CRITICAL, "failed to set uart%u_core_clk ret = %d\n", id, ret);
    ASSERT(0);
  }
}

/* Configure gpio for blsp uart */
VOID LibQcomPlatformUartDmGpioConfig(UINT8 id)
{
  /* configure rx gpio */
  gGpioTlmm->gpio_tlmm_config(5, 2, GPIO_INPUT, GPIO_NO_PULL, GPIO_8MA, GPIO_DISABLE);

  /* configure tx gpio */
  gGpioTlmm->gpio_tlmm_config(4, 2, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA, GPIO_DISABLE);
}
