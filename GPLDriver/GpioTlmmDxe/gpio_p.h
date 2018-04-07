#ifndef _GPIO_PRIVATE_H
#define _GPIO_PRIVATE_H

EFI_STATUS msm_gpio_direction_input(UINTN id);
EFI_STATUS msm_gpio_direction_output(UINTN id, UINTN value);
GPIO_DIRECTION msm_gpio_get_direction(UINTN id);
UINTN msm_gpio_get(UINTN id);
VOID msm_gpio_set(UINTN id, UINTN value);

EFI_STATUS msm_pinmux_set_function(UINTN id, UINTN function);
EFI_STATUS msm_pinmux_set_drive_strength(UINTN id, UINTN ma);
EFI_STATUS msm_pinmux_set_pull(UINTN id, GPIO_PULL pull);

/* APIs: exposed for other drivers */
/* API: Hdrive control for tlmm pins */
void tlmm_set_hdrive_ctrl(struct tlmm_cfgs *, uint8_t);
/* API:  Pull control for tlmm pins */
void tlmm_set_pull_ctrl(struct tlmm_cfgs *, uint8_t);

#endif // _GPIO_PRIVATE_H
