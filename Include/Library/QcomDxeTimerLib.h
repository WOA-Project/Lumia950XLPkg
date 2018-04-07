#ifndef __LIBRARY_QCOM_DXE_TIMER_LIB_H__
#define __LIBRARY_QCOM_DXE_TIMER_LIB_H__

VOID
EFIAPI
LibQcomDxeTimerEnable (
  UINT64 TimerTicks
  );

VOID
EFIAPI
LibQcomDxeTimerDisable (
  VOID
  );

UINTN
EFIAPI
LibQcomDxeTimerGetFreq (
  VOID
  );

VOID
EFIAPI
LibQcomDxeTimerFinishIrq (
  VOID
  );

#endif
