#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <GplLibrary/clock-rpm-msm8994.h>
#include <Protocol/QcomRpm.h>

#define RPM_MISC_CLK_TYPE 0x306b6c63
#define RPM_BUS_CLK_TYPE 0x316b6c63
#define RPM_MEM_CLK_TYPE 0x326b6c63
#define RPM_IPA_CLK_TYPE 0x617069
#define RPM_CE_CLK_TYPE 0x6563
#define RPM_MCFG_CLK_TYPE 0x6766636d

#define RPM_SMD_KEY_ENABLE 0x62616E45

#define CXO_CLK_SRC_ID 0x0
#define QDSS_CLK_ID 0x1

#define PNOC_CLK_ID 0x0
#define SNOC_CLK_ID 0x1
#define CNOC_CLK_ID 0x2
#define MMSSNOC_AHB_CLK_ID 0x3

#define BIMC_CLK_ID 0x0
#define GFX3D_CLK_SRC_ID 0x1
#define OCMEMGX_CLK_ID 0x2

#define IPA_CLK_ID 0x0

#define CE1_CLK_ID 0x0
#define CE2_CLK_ID 0x1
#define CE3_CLK_ID 0x2

#define MSS_CFG_AHB_CLK_ID 0x0

#define BB_CLK1_ID 0x1
#define BB_CLK2_ID 0x2
#define RF_CLK1_ID 0x4
#define RF_CLK2_ID 0x5
#define LN_BB_CLK_ID 0x8
#define DIV_CLK1_ID 0xb
#define DIV_CLK2_ID 0xc
#define DIV_CLK3_ID 0xd
#define BB_CLK1_PIN_ID 0x1
#define BB_CLK2_PIN_ID 0x2
#define RF_CLK1_PIN_ID 0x4
#define RF_CLK2_PIN_ID 0x5

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

#define RPM_SMD_KEY_RATE 0x007A484B
#define RPM_SMD_KEY_ENABLE 0x62616E45
#define RPM_SMD_KEY_STATE 0x54415453

#define RPM_CLK_BUFFER_A_REQ 0x616B6C63
#define RPM_KEY_SOFTWARE_ENABLE 0x6E657773
#define RPM_KEY_PIN_CTRL_CLK_BUFFER_ENABLE_KEY 0x62636370

#define GENERIC_DISABLE 0
#define GENERIC_ENABLE 1

#define RPM_CLK_BUFFER_PIN_CONTROL_ENABLE_NONE 0x0

/* Enable LN BB CLK */
static uint32_t ln_bb_clk[][8] = {
    {
        RPM_CLK_BUFFER_A_REQ,
        LN_BB_CLK_ID,
        RPM_KEY_SOFTWARE_ENABLE,
        4,
        GENERIC_DISABLE,
        RPM_KEY_PIN_CTRL_CLK_BUFFER_ENABLE_KEY,
        4,
        RPM_CLK_BUFFER_PIN_CONTROL_ENABLE_NONE,
    },
    {
        RPM_CLK_BUFFER_A_REQ,
        LN_BB_CLK_ID,
        RPM_KEY_SOFTWARE_ENABLE,
        4,
        GENERIC_ENABLE,
        RPM_KEY_PIN_CTRL_CLK_BUFFER_ENABLE_KEY,
        4,
        RPM_CLK_BUFFER_PIN_CONTROL_ENABLE_NONE,
    },
};

VOID EFIAPI rpm_smd_ln_bb_clk_enable(VOID)
{
  QCOM_RPM_PROTOCOL *RpmProtocol;
  EFI_STATUS         Status;

  Status =
      gBS->LocateProtocol(&gQcomRpmProtocolGuid, NULL, (VOID **)&RpmProtocol);

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to retrieve RPM protocol \n"));
    CpuDeadLoop();
  }

  RpmProtocol->rpm_clk_enable(&ln_bb_clk[GENERIC_ENABLE][0], 24);
  gBS->Stall(1000);
  DEBUG((EFI_D_INFO, "LN_BB_CLK enabled \n"));
}
