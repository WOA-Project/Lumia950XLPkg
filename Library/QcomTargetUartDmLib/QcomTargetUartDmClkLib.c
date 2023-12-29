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

