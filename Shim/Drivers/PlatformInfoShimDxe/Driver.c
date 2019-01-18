#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/PlatformInfoShim.h>

STATIC
EFI_STATUS
EFIAPI
PlatformInfoShimGetType(
	IN  EFI_PLATFORMINFO_SHIM_PROTOCOL           *This,
	OUT EFI_PLATFORMINFO_PLATFORM_INFO_TYPE		 *PlatformInfo
)
{
	if (PlatformInfo == NULL) return EFI_INVALID_PARAMETER;

	// This is a simple shim
	PlatformInfo->platform = EFI_PLATFORMINFO_TYPE_CHI;
	PlatformInfo->version = 2;
	PlatformInfo->subtype = 0;
	PlatformInfo->fusion = FALSE;

	return EFI_SUCCESS;
}

STATIC EFI_PLATFORMINFO_SHIM_PROTOCOL mPlatformInfoProtocol = {
	EFI_PLATFORMINFO_SHIM_PROTOCOL_VERSION,
	PlatformInfoShimGetType,
	NULL
};

EFI_STATUS
EFIAPI
PlatformInfoShimDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS Status;

	Status = gBS->InstallProtocolInterface(
		&ImageHandle,
		&gEfiPlatformInfoShimProtocolGuid,
		EFI_NATIVE_INTERFACE,
		(VOID*) &mPlatformInfoProtocol
	);

	ASSERT_EFI_ERROR(Status);

	return Status;
}