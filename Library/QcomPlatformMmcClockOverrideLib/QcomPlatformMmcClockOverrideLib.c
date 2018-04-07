#include <Base.h>
#include <Library/LKEnvLib.h>
#include <Library/QcomPlatformMmcClockOverrideLib.h>
#include <Chipset/mmc_sdhci.h>

UINT32 LibQcomPlatformMmcTranslateClockRate(UINT32 freq)
{
    if(freq == MMC_CLK_96MHZ)
    {
        return 100000000;
    }
    else if(freq == MMC_CLK_400MHZ)
    {
        return 384000000;
    }

    return freq;
}
