#ifndef __LIBRARY_QCOM_SSBI_LIB_H__
#define __LIBRARY_QCOM_SSBI_LIB_H__

#include <Protocol/QcomSsbi.h>

RETURN_STATUS
EFIAPI
SsbiImplLibInitialize(VOID);

extern QCOM_SSBI_PROTOCOL *gSSBI;

#endif
