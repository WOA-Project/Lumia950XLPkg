#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/QcomPcieInit.h>

EFI_STATUS
EFIAPI
QcomPCIeLoaderDxeEntry(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS Status;
	QCOM_PCIE_PROTOCOL *PCIExpressProtocol;

	DEBUG((EFI_D_ERROR | EFI_D_INFO, "Initialize PCIe RP\n"));

	Status = gBS->LocateProtocol(
		&gQcomPcieInitProtocolGuid,
		NULL,
		(VOID**) &PCIExpressProtocol
	);

	ASSERT(Status == EFI_SUCCESS);
	Status = PCIExpressProtocol->PCIeInitHardware(PCIExpressProtocol);
	ASSERT(Status == EFI_SUCCESS);

	DEBUG((EFI_D_ERROR | EFI_D_INFO, "PCIe RP initialized\n"));
	ASSERT(FALSE);

	return EFI_SUCCESS;
}