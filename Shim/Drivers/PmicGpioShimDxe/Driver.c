#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/PmicGpioShim.h>

STATIC
EFI_STATUS
EFIAPI
PmicGpioConfigDigitalInput(
	IN UINT32                                     PmicDeviceIndex,
	IN EFI_PM_GPIO_WHICH_TYPE                     Gpio,
	IN EFI_PM_GPIO_CURRENT_SOURCE_PULLS_TYPE      ISourcePulls,
	IN EFI_PM_GPIO_VOLTAGE_SOURCE_TYPE            VoltageSource,
	IN EFI_PM_GPIO_OUT_BUFFER_DRIVE_STRENGTH_TYPE OutBufferStrength,
	IN EFI_PM_GPIO_SOURCE_CONFIG_TYPE             Source
)
{
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
PmicGpioConfigDigitalOutput(
	IN UINT32                                     PmicDeviceIndex,
	IN EFI_PM_GPIO_WHICH_TYPE                     Gpio,
	IN EFI_PM_GPIO_OUT_BUFFER_CONFIG_TYPE         OutBufferConfig,
	IN EFI_PM_GPIO_VOLTAGE_SOURCE_TYPE            VoltageSource,
	IN EFI_PM_GPIO_SOURCE_CONFIG_TYPE             Source,
	IN EFI_PM_GPIO_OUT_BUFFER_DRIVE_STRENGTH_TYPE OutBufferStrength,
	IN BOOLEAN                                    OutInversion
)
{
	return EFI_SUCCESS;
}

STATIC EFI_QCOM_PMIC_GPIO_SHIM_PROTOCOL mPmicGpioProtocol = {
	PMIC_GPIO_SHIM_REVISION,
	PmicGpioConfigDigitalInput,
	PmicGpioConfigDigitalOutput
};

EFI_STATUS
EFIAPI
PmicGpioShimDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS Status;

	Status = gBS->InstallProtocolInterface(
		&ImageHandle,
		&gQcomPmicGpioShimProtocolGuid,
		EFI_NATIVE_INTERFACE,
		(VOID*) &mPmicGpioProtocol
	);

	ASSERT_EFI_ERROR(Status);

	return Status;
}