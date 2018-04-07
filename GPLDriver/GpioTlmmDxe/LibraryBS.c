#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/QcomGpioTlmmLib.h>
#include <Library/UefiBootServicesTableLib.h>

QCOM_GPIO_TLMM_PROTOCOL *gGpioTlmm = NULL;

RETURN_STATUS
EFIAPI
GpioTlmmLibConstructor (
  VOID
  )
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol (&gQcomGpioTlmmProtocolGuid, NULL, (VOID **)&gGpioTlmm);
  ASSERT_EFI_ERROR (Status);

  return Status;
}
