// TLMM Shim
// This is intended to maintain compatibility with Qualcomm binary module.
// Do not invoke if not necessary.

#ifndef __TLMM_SHIM_H__
#define __TLMM_SHIM_H__

#define DAL_GPIO_NUMBER(config)    (((config) >> 4) & 0x3FF)
#define DAL_GPIO_FUNCTION(config)   ((config) & 0xF)
#define DAL_GPIO_DIRECTION(config) (((config) >> 14) & 0x1)
#define DAL_GPIO_PULL(config)      (((config) >> 15) & 0x3)
#define DAL_GPIO_DRIVE(config)     (((config) >> 17) & 0xF)

#define EFI_TLMM_SHIM_PROTOCOL_REVISION 0x0000000000000001
#define EFI_TLMM_SHIM_PROTOCOL_GUID \
   { 0xad9aec18, 0x7bf0, 0x4809, {0x9e, 0x96, 0x30, 0x12, 0x30, 0x9f, 0x3d, 0xf7 } }

extern EFI_GUID gEfiTLMMShimProtocolGuid;

#define GPIO_ENUM_FORCE32(name) GPIO_##name##_SIZE = 0x7FFFFFFF
#define EFI_GPIO_CFG(gpio, func, dir, pull, drive) \
         (((gpio) & 0x3FF) << 4 | \
          ((func) & 0xF) |        \
          ((dir)  & 0x1) << 14|  \
          ((pull) & 0x3) << 15|  \
          ((drive)& 0xF) << 17| 0x20000000)

typedef struct _EFI_TLMM_SHIM_PROTOCOL EFI_TLMM_SHIM_PROTOCOL;

typedef enum
{
	TLMM_GPIO_DISABLE = 0,			/**< Set to low power. */
	TLMM_GPIO_ENABLE = 1,			/**< Set to a specified active state. */
	/** @cond */
	GPIO_ENUM_FORCE32(ENABLE)
	/** @endcond */
} TLMM_EnableType;

typedef enum
{
	GPIO_LOW_VALUE = 0,				/**< Set output to low. */
	GPIO_HIGH_VALUE = 1,			/**< Set output to high. */
	/** @cond */
	GPIO_ENUM_FORCE32(VALUE)
	/** @endcond */
} TLMM_ValueType;

typedef enum
{
	GPIO_INPUT = 0,					/**< Set to input. */
	GPIO_OUTPUT = 1,				/**< Set to output. */
	/** @cond */
	GPIO_ENUM_FORCE32(DIRECTION)
	/** @endcond */
} TLMM_DirectionType;

/**
Drive strength values. This enumeration specifies the drive strength
to use in a GPIO's configuration.
*/
/* see also  DAL_GPIO_CFG */
typedef enum
{
	GPIO_2MA = 0,					/**< 2 mA drive. */
	GPIO_4MA = 1,					/**< 4 mA drive. */
	GPIO_6MA = 2,					/**< 6 mA drive. */
	GPIO_8MA = 3,					/**< 8 mA drive. */
	GPIO_10MA = 4,					/**< 10 mA drive. */
	GPIO_12MA = 5,					/**< 12 mA drive. */
	GPIO_14MA = 6,					/**< 14 mA drive. */
	GPIO_16MA = 7,					/**< 16 mA drive. */
	/** @cond */
	GPIO_ENUM_FORCE32(DRIVESTR)
	/** @endcond */
} TLMM_DriveStrengthType;

typedef
EFI_STATUS
(EFIAPI *EFI_TLMM_CONFIG_GPIO)(
	IN UINT32 config,
	IN UINT32 enable
);

typedef
EFI_STATUS
(EFIAPI *EFI_TLMM_CONFIG_GPIO_GROUP)(
	IN UINT32 enable,
	IN UINT32 *config_group,
	IN UINT32 size
);

typedef
EFI_STATUS
(EFIAPI *EFI_TLMM_GPIO_IN)(
	IN UINT32 config,
	OUT UINT32 *value
);

typedef
EFI_STATUS
(EFIAPI *EFI_TLMM_GPIO_OUT)(
	IN UINT32 config,
	IN UINT32 value
);

typedef
EFI_STATUS
(EFIAPI *EFI_TLMM_SET_INACTIVE_CONFIG)(
	IN UINT32 gpio_number,
	IN UINT32 config
);

struct _EFI_TLMM_SHIM_PROTOCOL {
	UINT64                       Revision;
	EFI_TLMM_CONFIG_GPIO         ConfigGpio;
	EFI_TLMM_CONFIG_GPIO_GROUP   ConfigGpioGroup;
	EFI_TLMM_GPIO_IN             GpioIn;
	EFI_TLMM_GPIO_OUT            GpioOut;
	EFI_TLMM_SET_INACTIVE_CONFIG SetInactiveConfig;
};

#endif