#ifndef __LIBRARY_QCOM_TARGET_MMC_LIB_H__
#define __LIBRARY_QCOM_TARGET_MMC_LIB_H__

#include <Chipset/mmc.h>

typedef struct mmc_device *(*INIT_SLOT_CB)(UINT8 slot, UINTN base);

VOID LibQcomTargetMmcInit(INIT_SLOT_CB InitSlot);
VOID LibQcomTargetMmcCaps(struct mmc_host *host);

#endif
