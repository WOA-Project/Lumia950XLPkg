#ifndef __LIBRARY_QCOM_PM8X41_LIB_H__
#define __LIBRARY_QCOM_PM8X41_LIB_H__

#include <Protocol/QcomPm8x41.h>

RETURN_STATUS
EFIAPI
Pm8x41ImplLibInitialize (
  VOID
  );

extern QCOM_PM8X41_PROTOCOL *gPm8x41;

#endif
