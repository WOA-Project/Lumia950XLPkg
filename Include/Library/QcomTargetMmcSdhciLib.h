#ifndef __LIBRARY_QCOM_TARGET_MMC_LIB_H__
#define __LIBRARY_QCOM_TARGET_MMC_LIB_H__

#include <Chipset/mmc_sdhci.h>

typedef struct mmc_device *(*INIT_SLOT_CB)(struct mmc_config_data *config);

VOID LibQcomTargetMmcSdhciInit(INIT_SLOT_CB InitSlot);

#endif
