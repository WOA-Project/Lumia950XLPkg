#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/QcomGpioTlmm.h>
#include <Protocol/QcomGpioTlmmInterrupt.h>

#include <Protocol/TlmmShim.h>

QCOM_GPIO_TLMM_PROTOCOL* mTlmmProtocol;

STATIC
EFI_STATUS
EFIAPI
TlmmConfigGpio
(
	IN UINT32 Config,
	IN UINT32 Enable
)
{
	UINTN GpioNumber = DAL_GPIO_NUMBER(Config);
	UINTN GpioDirection = DAL_GPIO_DIRECTION(Config);

	if (GpioDirection == GPIO_INPUT)
	{
		mTlmmProtocol->DirectionInput(GpioNumber);
	}
	else
	{
		mTlmmProtocol->DirectionOutput(GpioNumber, Enable);
	}

	UINTN GpioFunction = DAL_GPIO_FUNCTION(Config);
	mTlmmProtocol->SetFunction(GpioNumber, GpioFunction);

	UINTN GpioPull = DAL_GPIO_PULL(Config);
	mTlmmProtocol->SetPull(GpioNumber, GpioPull);

	UINTN GpioDrive = DAL_GPIO_DRIVE(Config);
	switch (GpioDrive)
	{
	case GPIO_2MA:
		mTlmmProtocol->SetDriveStrength(GpioNumber, 2);
		break;
	case GPIO_4MA:
		mTlmmProtocol->SetDriveStrength(GpioNumber, 4);
		break;
	case GPIO_6MA:
		mTlmmProtocol->SetDriveStrength(GpioNumber, 6);
		break;
	case GPIO_8MA:
		mTlmmProtocol->SetDriveStrength(GpioNumber, 8);
		break;
	case GPIO_10MA:
		mTlmmProtocol->SetDriveStrength(GpioNumber, 10);
		break;
	case GPIO_12MA:
		mTlmmProtocol->SetDriveStrength(GpioNumber, 12);
		break;
	case GPIO_14MA:
		mTlmmProtocol->SetDriveStrength(GpioNumber, 14);
		break;
	case GPIO_16MA:
		mTlmmProtocol->SetDriveStrength(GpioNumber, 16);
		break;
	}

	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
TlmmConfigGpioGroup
(
	IN UINT32 Enable,
	IN UINT32 *ConfigGroup,
	IN UINT32 Size
)
{
	UINT32* Config = ConfigGroup;

	if (Config == NULL) return EFI_INVALID_PARAMETER;
	for (UINT32 i = 0; i < Size; i++)
	{
		TlmmConfigGpio(Config[i], Enable);
	}

	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
TlmmGpioIn(
	IN UINT32 Config,
	OUT UINT32 *Value
)
{
	UINTN GpioNumber = DAL_GPIO_NUMBER(Config);
	if (mTlmmProtocol->GetDirection(GpioNumber) == GPIO_DIRECTION_IN)
	{
		*Value = mTlmmProtocol->Get(GpioNumber);
		return EFI_SUCCESS;
	}

	return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
TlmmGpioOut(
	IN UINT32 Config,
	IN UINT32 Value
)
{
	UINTN GpioNumber = DAL_GPIO_NUMBER(Config);
	if (mTlmmProtocol->GetDirection(GpioNumber) == GPIO_DIRECTION_OUT)
	{
		mTlmmProtocol->Set(GpioNumber, Value);
		return EFI_SUCCESS;
	}
	
	return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
TlmmSetInactiveConfig
(
	IN UINT32 gpio_number,
	IN UINT32 config
)
{
	return EFI_SUCCESS;
}

STATIC EFI_TLMM_SHIM_PROTOCOL mTlmmShimProtocol = {
	EFI_TLMM_SHIM_PROTOCOL_REVISION,
	TlmmConfigGpio,
	TlmmConfigGpioGroup,
	TlmmGpioIn,
	TlmmGpioOut,
	TlmmSetInactiveConfig
};

EFI_STATUS
EFIAPI
TlmmShimDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS Status;

	Status = gBS->LocateProtocol(
		&gQcomGpioTlmmProtocolGuid,
		NULL,
		(VOID**) &mTlmmProtocol
	);

	// Only install the shim protocol when we are able to access TLMM implementation
	if (!EFI_ERROR(Status))
	{
		Status = gBS->InstallProtocolInterface(
			&ImageHandle,
			&gEfiTLMMShimProtocolGuid,
			EFI_NATIVE_INTERFACE,
			(VOID*) &mTlmmShimProtocol
		);

		ASSERT_EFI_ERROR(Status);
	}

	return Status;
}