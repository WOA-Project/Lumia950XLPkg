#include <Base.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomClockLib.h>
#include <Library/QcomGpioTlmmLib.h>
#include <Library/QcomTargetUartDmLib.h>

#include <Platform/iomap.h>

RETURN_STATUS LibQcomTargetUartCallSecConstructors(VOID)
{
  ClockImplLibInitialize();
  GpioTlmmImplLibInitialize();

  return RETURN_SUCCESS;
}

RETURN_STATUS
LibQcomTargetGetUartDmConfig(UINT8 *Id, UINTN *GsbiBase, UINTN *UartDmBase)
{
  *Id         = 2;
  *GsbiBase   = 0;
  *UartDmBase = BLSP1_UART1_BASE;

  return RETURN_SUCCESS;
}
