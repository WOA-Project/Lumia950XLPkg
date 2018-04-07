#ifndef __LIBRARY_QCOM_GPIO_TLMM_LIB_H__
#define __LIBRARY_QCOM_GPIO_TLMM_LIB_H__

#include <Protocol/QcomGpioTlmm.h>

RETURN_STATUS
EFIAPI
GpioTlmmImplLibInitialize (
  VOID
  );

extern QCOM_GPIO_TLMM_PROTOCOL *gGpioTlmm;

#endif
