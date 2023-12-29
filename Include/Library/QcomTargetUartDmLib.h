#ifndef __LIBRARY_QCOM_TARGET_UARTDM_LIB_H__
#define __LIBRARY_QCOM_TARGET_UARTDM_LIB_H__

RETURN_STATUS LibQcomTargetUartCallSecConstructors(VOID);
RETURN_STATUS LibQcomTargetGetUartDmConfig(UINT8 *Id, UINTN *GsbiBase, UINTN *UartDmBase);

#endif
