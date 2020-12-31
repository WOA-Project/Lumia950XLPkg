#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Chipset/regulator.h>
#include <Protocol/QcomRpm.h>

#include "rpm-ipc.h"
// Must come in order
#include "rpm-smd.h"

EFI_STATUS
EFIAPI
rpm_ldo_pipe_enable(VOID);

EFI_STATUS
EFIAPI
rpm_ldo30_enable(VOID);

STATIC QCOM_RPM_PROTOCOL mInternalRpm = {rpm_send_data, rpm_clk_enable,
                                         rpm_ldo_pipe_enable, rpm_ldo30_enable};

EFI_EVENT mExitBootServicesEvent;

static uint32_t ldo12[][11] = {
    {
        LDOA_RES_TYPE,
        12,
        KEY_SOFTWARE_ENABLE,
        4,
        GENERIC_DISABLE,
        KEY_MICRO_VOLT,
        4,
        0,
        KEY_CURRENT,
        4,
        0,
    },

    {
        LDOA_RES_TYPE,
        12,
        KEY_SOFTWARE_ENABLE,
        4,
        GENERIC_ENABLE,
        KEY_MICRO_VOLT,
        4,
        1800000,
        KEY_CURRENT,
        4,
        11,
    },
};

static uint32_t ldo28[][14] = {
    {
        LDOA_RES_TYPE,
        28,
        KEY_SOFTWARE_ENABLE,
        4,
        GENERIC_DISABLE,
        KEY_MICRO_VOLT,
        4,
        0,
        KEY_CURRENT,
        4,
        0,
    },

    {
        LDOA_RES_TYPE,
        28,
        KEY_SOFTWARE_ENABLE,
        4,
        GENERIC_ENABLE,
        KEY_MICRO_VOLT,
        4,
        1000000,
        KEY_CURRENT,
        4,
        72,
    },
};

static uint32_t ldo30[][14] = {
    {
        LDOA_RES_TYPE,
        30,
        KEY_SOFTWARE_ENABLE,
        4,
        GENERIC_DISABLE,
        KEY_MICRO_VOLT,
        4,
        0,
        KEY_CURRENT,
        4,
        0,
    },

    {
        LDOA_RES_TYPE,
        30,
        KEY_SOFTWARE_ENABLE,
        4,
        GENERIC_ENABLE,
        KEY_MICRO_VOLT,
        4,
        1800000,
        KEY_CURRENT,
        4,
        0,
    },
};

static uint32_t bst5v[][14] = {
    {
        BTSB_RES_TYPE,
        1,
        KEY_SOFTWARE_ENABLE,
        4,
        GENERIC_DISABLE,
        KEY_MICRO_VOLT,
        4,
        0,
        KEY_CURRENT,
        4,
        0,
    },
    {
        BTSB_RES_TYPE,
        1,
        KEY_SOFTWARE_ENABLE,
        4,
        GENERIC_ENABLE,
        KEY_MICRO_VOLT,
        4,
        5000000,
        KEY_CURRENT,
        4,
        0,
    },
};

VOID EFIAPI RpmDxeDeInitialize(IN EFI_EVENT Event, IN VOID *Context)
{
  DEBUG((EFI_D_INFO, "Unregistering RPM \n"));

  // Call ExitBS version SMD uninit routine
  // Which has no memory ops and events
  rpm_smd_uninit_exit_bs();

  DEBUG((EFI_D_INFO, "Unregistering RPM Completed \n"));
}

EFI_STATUS
EFIAPI
rpm_ldo_pipe_enable(VOID)
{
  // Also enable PM8994 LDO12, LDO28
  ASSERT(rpm_send_data(&ldo12[GENERIC_ENABLE][0], 36, RPM_REQUEST_TYPE) == 0);
  gBS->Stall(100);
  DEBUG((EFI_D_INFO, "LDO12 enabled \n"));

  ASSERT(rpm_send_data(&ldo28[GENERIC_ENABLE][0], 36, RPM_REQUEST_TYPE) == 0);
  gBS->Stall(100);
  DEBUG((EFI_D_INFO, "LDO28 enabled \n"));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
rpm_ldo30_enable(VOID)
{
  // LDO30
  ASSERT(rpm_send_data(&ldo30[GENERIC_ENABLE][0], 36, RPM_REQUEST_TYPE) == 0);
  gBS->Stall(100);
  DEBUG((EFI_D_INFO, "LDO30 enabled \n"));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
rpm_5vsw_enable(VOID)
{
  // Unconditionally enable 5V Boost (BTSB)
  ASSERT(rpm_send_data(&bst5v[GENERIC_ENABLE][0], 36, RPM_REQUEST_TYPE) == 0);
  gBS->Stall(100);
  DEBUG((EFI_D_INFO, "5V BTSB enabled \n"));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RpmDxeInitialize(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_HANDLE Handle = NULL;
  EFI_STATUS Status = EFI_SUCCESS;

  // Init RPM SMD channel
  rpm_smd_init();

  // Try to initialize 5V boost circuit
  rpm_5vsw_enable();

  // Since RPM usage is pretty limited in current UEFI
  // The protocol will not be installed unless the flag is set.
  // As for PCIe clocks, it is pre-enabled in BootShim
  if (FeaturePcdGet(PcdInstallRpmProtocol)) {
    Status = gBS->InstallMultipleProtocolInterfaces(
        &Handle, &gQcomRpmProtocolGuid, &mInternalRpm, NULL);
    ASSERT_EFI_ERROR(Status);

    // Register Exit BS event for RPM SMD uninit.
    // Otherwise Windows will hang at startup.
    Status = gBS->CreateEventEx(
        EVT_NOTIFY_SIGNAL, TPL_NOTIFY, RpmDxeDeInitialize, NULL,
        &gEfiEventExitBootServicesGuid, &mExitBootServicesEvent);

    ASSERT_EFI_ERROR(Status);
  }
  else {
    // Immediately unload RPM
    rpm_smd_uninit();
  }

  return Status;
}
