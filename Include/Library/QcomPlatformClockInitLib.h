#ifndef __LIBRARY_QCOM_PLATFORM_CLOCKINIT_LIB_H__
#define __LIBRARY_QCOM_PLATFORM_CLOCKINIT_LIB_H__

#include <Chipset/clock.h>
#include <Library/LKEnvLib.h>

EFI_STATUS
EFIAPI
LibQcomPlatformClockInit(struct clk_lookup **clist, unsigned *num);

#endif
