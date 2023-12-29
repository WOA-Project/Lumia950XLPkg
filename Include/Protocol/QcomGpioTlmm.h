#ifndef __QCOM_PROTOCOL_GPIO_TLMM_H__
#define __QCOM_PROTOCOL_GPIO_TLMM_H__

#include <Chipset/gpio.h>

#define QCOM_GPIO_TLMM_PROTOCOL_GUID                                           \
  {                                                                            \
    0x8054947b, 0x3223, 0x407a,                                                \
    {                                                                          \
      0xa1, 0xcc, 0x31, 0x22, 0x2f, 0x80, 0x66, 0x40                           \
    }                                                                          \
  }

typedef struct _QCOM_GPIO_TLMM_PROTOCOL QCOM_GPIO_TLMM_PROTOCOL;

typedef enum {
  GPIO_DIRECTION_OUT = 0,
  GPIO_DIRECTION_IN  = 1,
} GPIO_DIRECTION;

typedef enum {
  GPIO_PULL_NONE   = 0,
  GPIO_PULL_DOWN   = 1,
  GPIO_PULL_KEEPER = 2,
  GPIO_PULL_UP     = 3,
} GPIO_PULL;

#define GPIO_LOW 0
#define GPIO_HIGH 1

// Referenced from gpio.h
/* GPIO TLMM: Direction */
#define GPIO_INPUT      0
#define GPIO_OUTPUT     1

/* GPIO TLMM: Pullup/Pulldown */
#define GPIO_NO_PULL    0
#define GPIO_PULL_DOWN  1
#define GPIO_KEEPER     2
#define GPIO_PULL_UP    3

/* GPIO TLMM: Drive Strength */
#define GPIO_2MA        0
#define GPIO_4MA        1
#define GPIO_6MA        2
#define GPIO_8MA        3
#define GPIO_10MA       4
#define GPIO_12MA       5
#define GPIO_14MA       6
#define GPIO_16MA       7

/* GPIO TLMM: Status */
#define GPIO_ENABLE     0
#define GPIO_DISABLE    1

typedef EFI_STATUS(EFIAPI *msm_gpio_direction_input_t)(UINTN id);
typedef EFI_STATUS(EFIAPI *msm_gpio_direction_output_t)(UINTN id, UINTN value);
typedef GPIO_DIRECTION(EFIAPI *msm_gpio_get_direction_t)(UINTN id);
typedef UINTN(EFIAPI *msm_gpio_get_t)(UINTN id);
typedef VOID(EFIAPI *msm_gpio_set_t)(UINTN id, UINTN value);

typedef EFI_STATUS(EFIAPI *msm_pinmux_set_function_t)(UINTN id, UINTN function);
typedef EFI_STATUS(EFIAPI *msm_pinmux_set_drive_strength_t)(UINTN id, UINTN ma);
typedef EFI_STATUS(EFIAPI *msm_pinmux_set_pull_t)(UINTN id, GPIO_PULL pull);

typedef void(EFIAPI *tlmm_set_hdrive_ctrl_t)(struct tlmm_cfgs *, uint8_t);
typedef void(EFIAPI *tlmm_set_pull_ctrl_t)(struct tlmm_cfgs *, uint8_t);

typedef void(EFIAPI *msm_gpio_tlmm_config)(UINT32 gpio, UINT8 func,
		      UINT8 dir, UINT8 pull,
		      UINT8 drvstr, UINT32 enable);

struct _QCOM_GPIO_TLMM_PROTOCOL {
  msm_gpio_direction_input_t  DirectionInput;
  msm_gpio_direction_output_t DirectionOutput;
  msm_gpio_get_direction_t    GetDirection;
  msm_gpio_get_t              Get;
  msm_gpio_set_t              Set;

  msm_pinmux_set_function_t       SetFunction;
  msm_pinmux_set_drive_strength_t SetDriveStrength;
  msm_pinmux_set_pull_t           SetPull;

  tlmm_set_hdrive_ctrl_t tlmm_set_hdrive_ctrl;
  tlmm_set_pull_ctrl_t   tlmm_set_pull_ctrl;

  msm_gpio_tlmm_config        gpio_tlmm_config;
};

extern EFI_GUID gQcomGpioTlmmProtocolGuid;

#endif
