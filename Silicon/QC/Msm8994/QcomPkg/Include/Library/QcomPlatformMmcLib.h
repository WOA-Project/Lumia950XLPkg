#ifndef __LIBRARY_QCOM_PLATFORM_MMC_LIB_H__
#define __LIBRARY_QCOM_PLATFORM_MMC_LIB_H__

VOID LibQcomPlatformMmcClockInit(UINT32 interface);
VOID LibQcomPlatformMmcClockConfig(UINT32 interface, UINT32 freq);
VOID LibQcomPlatformMmcClockConfigCdc(UINT32 interface);

#endif
