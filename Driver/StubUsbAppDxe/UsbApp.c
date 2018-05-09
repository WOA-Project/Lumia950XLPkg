#include <PiDxe.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Protocol/EFIDroidUsbDevice.h>

USB_DEVICE_PROTOCOL* DeviceProtocol;

EFI_STATUS
EFIAPI
UsbAppEntryPoint(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS Status = EFI_SUCCESS;
	USB_DEVICE_DESCRIPTOR UsbDeviceDescriptor;
	
	DEBUG((EFI_D_ERROR, "Starting USB... \n"));
	Status = gBS->LocateProtocol(
		&gEFIDroidUsbDeviceProtocolGuid,
		NULL,
		(VOID*) &DeviceProtocol
	);

	if (EFI_ERROR(Status))
	{
		DEBUG((EFI_D_ERROR, "Failed to locate protocol. \n"));
		goto exit;
	}

exit:
	return Status;

}