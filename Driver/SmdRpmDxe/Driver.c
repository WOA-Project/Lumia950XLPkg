#include <PiDxe.h>
#include <Library/LKEnvLib.h>

#include <Protocol/QcomRpm.h>
#include <Library/UefiBootServicesTableLib.h>

#include "rpm-ipc.h"
#include "rpm-smd.h"

STATIC QCOM_RPM_PROTOCOL mInternalRpm = {
  rpm_send_data,
  rpm_clk_enable,
};

EFI_EVENT mExitBootServicesEvent;

VOID
EFIAPI
RpmDxeDeInitialize(
	IN EFI_EVENT        Event,
	IN VOID             *Context
)
{
	DEBUG((EFI_D_INFO, "Unregistering RPM \n"));

	// Call ExitBS version SMD uninit routine
	// Which has no memory ops and events
	rpm_smd_uninit_exit_bs();

	DEBUG((EFI_D_INFO, "Unregistering RPM Completed \n"));
}

EFI_STATUS
EFIAPI
RpmDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
	EFI_HANDLE Handle = NULL;
	EFI_STATUS Status;

	rpm_smd_init();

	Status = gBS->InstallMultipleProtocolInterfaces(
		&Handle,
		&gQcomRpmProtocolGuid,      
		&mInternalRpm,
		NULL
	);
	ASSERT_EFI_ERROR(Status);

	// Register Exit BS event for RPM SMD uninit.
	// Otherwise Windows will hang at startup.

	Status = gBS->CreateEventEx(
		EVT_NOTIFY_SIGNAL,
		TPL_NOTIFY,
		RpmDxeDeInitialize,
		NULL,
		&gEfiEventExitBootServicesGuid,
		&mExitBootServicesEvent
	);

	ASSERT_EFI_ERROR(Status);

	return Status;
}
