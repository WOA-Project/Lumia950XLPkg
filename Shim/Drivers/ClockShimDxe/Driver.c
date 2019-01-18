#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/ClockShim.h>

STATIC
EFI_STATUS
EFIAPI
ClkShimGetClockId(
	IN  EFI_CLOCK_SHIM_PROTOCOL *This,
	IN  CONST CHAR8        *ClockName,
	OUT UINTN              *ClockId
)
{
	if (ClockName == NULL || ClockId == NULL) return EFI_INVALID_PARAMETER;

	// This is a simple shim
	*ClockId = 0x100;

	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimEnableClock(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN UINTN              ClockId
)
{
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimDisableClock(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN UINTN              ClockId
)
{
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimIsClockEnabled(
	IN  EFI_CLOCK_SHIM_PROTOCOL *This,
	IN  UINTN              ClockId,
	OUT BOOLEAN            *IsEnabled
)
{
	if (IsEnabled == NULL) return EFI_INVALID_PARAMETER;

	*IsEnabled = TRUE;
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimIsClockOn(
	IN  EFI_CLOCK_SHIM_PROTOCOL *This,
	IN  UINTN              ClockId,
	OUT BOOLEAN            *IsOn
)
{
	if (IsOn == NULL) return EFI_INVALID_PARAMETER;

	*IsOn = TRUE;
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimSetFrequency(
	IN  EFI_CLOCK_SHIM_PROTOCOL        *This,
	IN  UINTN                     ClockId,
	IN  UINT32                    Freq,
	IN  EFI_CLOCK_FREQUENCY_TYPE  Match,
	OUT UINT32                    *ResultFreq
)
{
	if (ResultFreq == NULL) return EFI_INVALID_PARAMETER;
	*ResultFreq = Freq;

	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimGetFrequency(
	IN  EFI_CLOCK_SHIM_PROTOCOL *This,
	IN  UINTN              ClockId,
	OUT UINT32             *FreqHz
)
{
	if (FreqHz == NULL) return EFI_INVALID_PARAMETER;
	*FreqHz = 1920000;

	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimCalcFrequency(
	IN  EFI_CLOCK_SHIM_PROTOCOL *This,
	IN  UINTN              ClockId,
	OUT UINT32             *FreqHz
)
{
	if (FreqHz == NULL) return EFI_INVALID_PARAMETER;
	*FreqHz = 1920000;

	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimSelectExternalSource(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN UINTN              ClockId,
	IN UINT32             FreqHz,
	IN UINT32             Source,
	IN UINT32             Divider,
	IN UINT32             VAL_M,
	IN UINT32             VAL_N,
	IN UINT32             VAL_2D
)
{
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimGetPowerDomainId(
	IN  EFI_CLOCK_SHIM_PROTOCOL *This,
	IN  CONST CHAR8        *ClockPowerDomainName,
	OUT UINTN              *ClockPowerDomainId
)
{
	if (ClockPowerDomainName == NULL || ClockPowerDomainId == NULL) return EFI_INVALID_PARAMETER;

	*ClockPowerDomainId = 0;

	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimEnablePowerDomain(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN UINTN              ClockPowerDomainId
)
{
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimDisablePowerDomain(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN UINTN              ClockPowerDomainId
)
{
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimPowerDomainEnterLpm(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN EFI_CLOCK_LPM_TYPE lpmtype
)
{
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimPowerDomainExitLpm(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN EFI_CLOCK_LPM_TYPE lpmtype
)
{
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimSetClockDivider(
	IN  EFI_CLOCK_SHIM_PROTOCOL        *This,
	IN  UINTN                     ClockId,
	IN  UINT32                    Divider
)
{
	return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ClkShimResetClock(
	IN EFI_CLOCK_SHIM_PROTOCOL    *This,
	IN UINTN					  ClockId,
	IN EFI_CLOCK_RESET_TYPE       eReset
)
{
	return EFI_SUCCESS;
}

STATIC EFI_CLOCK_SHIM_PROTOCOL mClockShimProtocol = {
	EFI_CLOCK_SHIM_PROTOCOL_VERSION,
	ClkShimGetClockId,
	ClkShimEnableClock,
	ClkShimDisableClock,
	ClkShimIsClockEnabled,
	ClkShimIsClockOn,
	ClkShimSetFrequency,
	ClkShimGetFrequency,
	ClkShimCalcFrequency,
	ClkShimSelectExternalSource,
	ClkShimGetPowerDomainId,
	ClkShimEnablePowerDomain,
	ClkShimDisablePowerDomain,
	ClkShimPowerDomainEnterLpm,
	ClkShimPowerDomainExitLpm,
	ClkShimSetClockDivider,
	ClkShimResetClock
};

EFI_STATUS
EFIAPI
ClockShimDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS Status;

	Status = gBS->InstallProtocolInterface(
		&ImageHandle,
		&gEfiClockShimProtocolGuid,
		EFI_NATIVE_INTERFACE,
		(VOID*) &mClockShimProtocol
	);

	ASSERT_EFI_ERROR(Status);

	return Status;
}