#include <PiDxe.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/QcomGpioTlmmInterrupt.h>
#include <Library/DebugLib.h>

#include "gpio_driver_p.h"

STATIC QCOM_GPIO_TLMM_INTERRUPT_PROTOCOL mGpioTlmmInterrupt = {
  msm_gpio_irq_mask,
  msm_gpio_irq_unmask,
  msm_gpio_irq_ack,
  msm_gpio_irq_set_type,
  msm_gpio_irq_set_custom_handler,
};


EFI_STATUS
EFIAPI
GpioTlmmInterruptDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_HANDLE Handle = NULL;
  EFI_STATUS Status;

  msm_gpio_irq_init();

  Status = gBS->InstallMultipleProtocolInterfaces(
                  &Handle,
                  &gQcomGpioTlmmInterruptProtocolGuid,      &mGpioTlmmInterrupt,
                  NULL
                  );
  ASSERT_EFI_ERROR(Status);

  return Status;
}
