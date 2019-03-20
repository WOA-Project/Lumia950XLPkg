#ifndef __LIBRARY_QCOM_RPM_LIB_H__
#define __LIBRARY_QCOM_RPM_LIB_H__

#include <Protocol/QcomRpm.h>

RETURN_STATUS
EFIAPI
RpmImplLibInitialize(VOID);

extern QCOM_RPM_PROTOCOL *gRpm;

#endif
