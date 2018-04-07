#ifndef __LIBRARY_QCOM_SPMI_LIB_H__
#define __LIBRARY_QCOM_SPMI_LIB_H__

#include <Protocol/QcomSpmi.h>

RETURN_STATUS
EFIAPI
SpmiImplLibInitialize (
  VOID
  );

extern QCOM_SPMI_PROTOCOL *gSpmi;

#endif
