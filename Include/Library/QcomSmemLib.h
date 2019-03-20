#ifndef __LIBRARY_QCOM_SMEM_LIB_H__
#define __LIBRARY_QCOM_SMEM_LIB_H__

#include <Protocol/QcomSmem.h>

RETURN_STATUS
EFIAPI
SmemImplLibInitialize(VOID);

extern QCOM_SMEM_PROTOCOL *gSMEM;

#endif
