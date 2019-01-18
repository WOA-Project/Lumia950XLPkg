#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/NpaShim.h>

STATIC
EFI_STATUS
EFIAPI
NpaShimCreateSyncClientEx(
	const char        *resource_name,
	const char        *client_name,
	npa_client_type   client_type,
	unsigned int      client_value,
	void              *client_ref,
	npa_client_handle *client_handle
)
{
	if (client_handle == NULL) return EFI_INVALID_PARAMETER;

	*client_handle = (void*) 0x100;
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
NpmShimCompleteRequest(
	npa_client_handle client
)
{
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
NpmShimDestroyClient(
	npa_client_handle client
)
{
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
NpmShimScalarRequest(
	npa_client_handle   client,
	npa_resource_state  state
)
{
	return EFI_SUCCESS;
}

STATIC EFI_NPA_PROTOCOL mNpaProtocol = 
{
	EFI_NPA_SHIM_PROTOCOL_REVISION,
	NULL,
	NpaShimCreateSyncClientEx,
	NpmShimCompleteRequest,
	NULL,
	NpmShimScalarRequest,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NpmShimDestroyClient
};

EFI_STATUS
EFIAPI
NpaShimDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS Status;

	Status = gBS->InstallProtocolInterface(
		&ImageHandle,
		&gEfiNpaShimProtocolGuid,
		EFI_NATIVE_INTERFACE,
		(VOID*) &mNpaProtocol
	);

	ASSERT_EFI_ERROR(Status);

	return Status;
}
