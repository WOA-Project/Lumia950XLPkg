#include <Base.h>
#include <Platform/iomap.h>

RETURN_STATUS
LibQcomTargetGetUartDmConfig(UINT8 *Id, UINTN *GsbiBase, UINTN *UartDmBase)
{
  *Id         = 2;
  *GsbiBase   = 0;
  // Note: this is actually blsp1_uart2
  *UartDmBase = BLSP1_UART1_BASE;

  return RETURN_SUCCESS;
}
