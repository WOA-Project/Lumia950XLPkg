#ifndef __QCOM_PROTOCOL_GPIO_TLMM_H__
#define __QCOM_PROTOCOL_GPIO_TLMM_H__

#include <Chipset/gpio.h>

#define QCOM_GPIO_TLMM_PROTOCOL_GUID \
  { 0x8054947b, 0x3223, 0x407a, { 0xa1, 0xcc, 0x31, 0x22, 0x2f, 0x80, 0x66, 0x40 } }

typedef struct _QCOM_GPIO_TLMM_PROTOCOL   QCOM_GPIO_TLMM_PROTOCOL;

typedef enum {
  GPIO_DIRECTION_OUT = 0,
  GPIO_DIRECTION_IN = 1,
} GPIO_DIRECTION;

typedef enum {
  GPIO_PULL_NONE = 0,
  GPIO_PULL_DOWN = 1,
  GPIO_PULL_KEEPER = 2,
  GPIO_PULL_UP = 3,
} GPIO_PULL;

typedef EFI_STATUS (EFIAPI *msm_gpio_direction_input_t)(UINTN id);
typedef EFI_STATUS (EFIAPI *msm_gpio_direction_output_t)(UINTN id, UINTN value);
typedef GPIO_DIRECTION (EFIAPI *msm_gpio_get_direction_t)(UINTN id);
typedef UINTN (EFIAPI *msm_gpio_get_t)(UINTN id);
typedef VOID (EFIAPI *msm_gpio_set_t)(UINTN id, UINTN value);

typedef EFI_STATUS (EFIAPI *msm_pinmux_set_function_t)(UINTN id, UINTN function);
typedef EFI_STATUS (EFIAPI *msm_pinmux_set_drive_strength_t)(UINTN id, UINTN ma);
typedef EFI_STATUS (EFIAPI *msm_pinmux_set_pull_t)(UINTN id, GPIO_PULL pull);

typedef void (EFIAPI *tlmm_set_hdrive_ctrl_t)(struct tlmm_cfgs *, uint8_t);
typedef void (EFIAPI *tlmm_set_pull_ctrl_t)(struct tlmm_cfgs *, uint8_t);

struct _QCOM_GPIO_TLMM_PROTOCOL {
  msm_gpio_direction_input_t          DirectionInput;
  msm_gpio_direction_output_t         DirectionOutput;
  msm_gpio_get_direction_t            GetDirection;
  msm_gpio_get_t                      Get;
  msm_gpio_set_t                      Set;

  msm_pinmux_set_function_t           SetFunction;
  msm_pinmux_set_drive_strength_t     SetDriveStrength;
  msm_pinmux_set_pull_t               SetPull;

  tlmm_set_hdrive_ctrl_t     tlmm_set_hdrive_ctrl;
  tlmm_set_pull_ctrl_t       tlmm_set_pull_ctrl;
};

extern EFI_GUID gQcomGpioTlmmProtocolGuid;

#endif
