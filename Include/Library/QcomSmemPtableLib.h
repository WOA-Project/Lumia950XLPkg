#ifndef __LIBRARY_QCOM_SMEM_PTABLE_LIB_H__
#define __LIBRARY_QCOM_SMEM_PTABLE_LIB_H__

#include <Protocol/QcomSmemPtable.h>

RETURN_STATUS
EFIAPI
SmemPtableImplLibInitialize(VOID);

extern QCOM_SMEM_PTABLE_PROTOCOL *gSmemPtable;

#endif
