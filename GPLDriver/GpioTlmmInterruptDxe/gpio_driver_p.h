#ifndef _GPIO_DRIVER_PRIVATE_H
#define _GPIO_DRIVER_PRIVATE_H

VOID msm_gpio_irq_init(VOID);
VOID msm_gpio_irq_mask(UINTN id);
VOID msm_gpio_irq_unmask(UINTN id);
VOID msm_gpio_irq_ack(UINTN id);
EFI_STATUS msm_gpio_irq_set_type(UINTN id, UINTN type);
VOID msm_gpio_irq_set_custom_handler(UINTN id, GPIO_INTERRUPT_HANDLER handler);

#endif // _GPIO_PRIVATE_H
