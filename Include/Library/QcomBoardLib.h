#ifndef __LIBRARY_QCOM_BOARD_LIB_H__
#define __LIBRARY_QCOM_BOARD_LIB_H__

#include <Protocol/QcomBoard.h>

RETURN_STATUS
EFIAPI
BoardImplLibInitialize(VOID);

extern QCOM_BOARD_PROTOCOL *gBoard;

#endif
