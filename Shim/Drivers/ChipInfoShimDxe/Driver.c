#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/ChipInfoShim.h>

STATIC
EFI_STATUS
EFIAPI
DalChipInfoShimGetChipId(
	IN EFI_CHIPINFO_SHIM_PROTOCOL *This,
	OUT EFIChipInfoIdType *peId
)
{
	if (peId == NULL) return EFI_INVALID_PARAMETER;

	// This is a simple shim
	*peId = EFICHIPINFO_ID_MSM8994;

	return EFI_SUCCESS;
}

STATIC EFI_CHIPINFO_SHIM_PROTOCOL mChipInfoProtocol = {
	EFI_CHIPINFO_PROTOCOL_REVISION,
	NULL,
	NULL,
	DalChipInfoShimGetChipId
};

EFI_STATUS
EFIAPI
ChipInfoShimDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS Status;

	Status = gBS->InstallProtocolInterface(
		&ImageHandle,
		&gEfiChipInfoShimProtocolGuid,
		EFI_NATIVE_INTERFACE,
		(VOID*) &mChipInfoProtocol
	);

	ASSERT_EFI_ERROR(Status);

	return Status;
}