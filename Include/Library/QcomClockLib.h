#ifndef __LIBRARY_QCOM_CLOCK_LIB_H__
#define __LIBRARY_QCOM_CLOCK_LIB_H__

#include <Protocol/QcomClock.h>

RETURN_STATUS
EFIAPI
ClockImplLibInitialize (
  VOID
  );

extern QCOM_CLOCK_PROTOCOL *gClock;

#endif
