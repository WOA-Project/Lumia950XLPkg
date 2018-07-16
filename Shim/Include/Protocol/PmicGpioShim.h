// PMIC GPIO Shim
// This is intended to maintain compatibility with Qualcomm binary module.
// Do not invoke if not necessary.

#ifndef __PMIC_GPIO_SHIM_PROTOCOL_H__
#define __PMIC_GPIO_SHIM_PROTOCOL_H__

#define PMIC_GPIO_SHIM_REVISION 0x0000000000010003
#define EFI_PMIC_GPIO_SHIM_PROTOCOL_GUID \
	{ 0x60759b13, 0xa8bf, 0x46fe, { 0xb7, 0xe6, 0x79, 0x7b, 0xfb, 0x33, 0x5d, 0xf3 } }

extern EFI_GUID gQcomPmicGpioShimProtocolGuid;

typedef struct _EFI_QCOM_PMIC_GPIO_SHIM_PROTOCOL   EFI_QCOM_PMIC_GPIO_SHIM_PROTOCOL;

typedef enum
{
	EFI_PM_GPIO_1,  /**< GPIO 1. */
	EFI_PM_GPIO_2,  /**< GPIO 2. */
	EFI_PM_GPIO_3,  /**< GPIO 3. */
	EFI_PM_GPIO_4,  /**< GPIO 4. */
	EFI_PM_GPIO_5,  /**< GPIO 5. */
	EFI_PM_GPIO_6,  /**< GPIO 6. */
	EFI_PM_GPIO_7,  /**< GPIO 7. */
	EFI_PM_GPIO_8,  /**< GPIO 8. */
	EFI_PM_GPIO_9,  /**< GPIO 9. */
	EFI_PM_GPIO_10, /**< GPIO 10. */
	EFI_PM_GPIO_11, /**< GPIO 11. */
	EFI_PM_GPIO_12, /**< GPIO 12. */
	EFI_PM_GPIO_13, /**< GPIO 13. */
	EFI_PM_GPIO_14, /**< GPIO 14. */
	EFI_PM_GPIO_15, /**< GPIO 15. */
	EFI_PM_GPIO_16, /**< GPIO 16. */
	EFI_PM_GPIO_17, /**< GPIO 17. */
	EFI_PM_GPIO_18, /**< GPIO 18. */
	EFI_PM_GPIO_19, /**< GPIO 19. */
	EFI_PM_GPIO_20, /**< GPIO 20. */
	EFI_PM_GPIO_21, /**< GPIO 21. */
	EFI_PM_GPIO_22, /**< GPIO 22. */
	EFI_PM_GPIO_23, /**< GPIO 23. */
	EFI_PM_GPIO_24, /**< GPIO 24. */
	EFI_PM_GPIO_25, /**< GPIO 25. */
	EFI_PM_GPIO_26, /**< GPIO 26. */
	EFI_PM_GPIO_27, /**< GPIO 27. */
	EFI_PM_GPIO_28, /**< GPIO 28. */
	EFI_PM_GPIO_29, /**< GPIO 29. */
	EFI_PM_GPIO_30, /**< GPIO 30. */
	EFI_PM_GPIO_31, /**< GPIO 31. */
	EFI_PM_GPIO_32, /**< GPIO 32. */
	EFI_PM_GPIO_33, /**< GPIO 33. */
	EFI_PM_GPIO_34, /**< GPIO 34. */
	EFI_PM_GPIO_35, /**< GPIO 35. */
	EFI_PM_GPIO_36, /**< GPIO 36. */
	EFI_PM_GPIO_37, /**< GPIO 37. */
	EFI_PM_GPIO_38, /**< GPIO 38. */
	EFI_PM_GPIO_39, /**< GPIO 39. */
	EFI_PM_GPIO_40, /**< GPIO 40. */
	EFI_PM_GPIO_41, /**< GPIO 41. */
	EFI_PM_GPIO_42, /**< GPIO 42. */
	EFI_PM_GPIO_43, /**< GPIO 43. */
	EFI_PM_GPIO_44  /**< GPIO 44. */
} EFI_PM_GPIO_WHICH_TYPE;

typedef enum
{
	EFI_PM_GPIO_VIN0, /**< VIN0. */
	EFI_PM_GPIO_VIN1, /**< VIN1. */
	EFI_PM_GPIO_VIN2, /**< VIN2. */
	EFI_PM_GPIO_VIN3, /**< VIN3. */
	EFI_PM_GPIO_VIN4, /**< VIN4. */
	EFI_PM_GPIO_VIN5, /**< VIN5. */
	EFI_PM_GPIO_VIN6, /**< VIN6. */
	EFI_PM_GPIO_VIN7, /**< VIN7. */
	EFI_PM_GPIO_VOLTAGE_SOURCE_TYPE__INVALID /**< Invalid source. */
} EFI_PM_GPIO_VOLTAGE_SOURCE_TYPE;

typedef enum
{
	EFI_PM_GPIO_OUT_BUFFER_CONFIG_CMOS,       /**< CMOS. */
	EFI_PM_GPIO_OUT_BUFFER_CONFIG_OPEN_DRAIN, /**< Open drain. */
	EFI_PM_GPIO_OUT_BUFFER_CONFIG_INVALID     /**< Invalid buffer configuration. */
} EFI_PM_GPIO_OUT_BUFFER_CONFIG_TYPE;

typedef enum
{
	EFI_PM_GPIO_I_SOURCE_PULL_UP_30uA,
	/**< Pull up is 30 microampere. */
	EFI_PM_GPIO_I_SOURCE_PULL_UP_1_5uA,
	/**< Pull up is 1.5 microampere. */
	EFI_PM_GPIO_I_SOURCE_PULL_UP_31_5uA,
	/**< Pull up is 31.5 microampere. */
	EFI_PM_GPIO_I_SOURCE_PULL_UP_1_5uA_PLUS_30uA_BOOST,
	/**< Pull up is 1.5 microampere plus 30 microampere boost. */
	EFI_PM_GPIO_I_SOURCE_PULL_DOWN_10uA,
	/**< Pull down is 10 microampere. */
	EFI_PM_GPIO_I_SOURCE_PULL_NO_PULL,
	/**< No pull. */
	EFI_PM_GPIO_CURRENT_SOURCE_PULLS_TYPE__INVALID
	/**< Invalid pull. @newpage */
} EFI_PM_GPIO_CURRENT_SOURCE_PULLS_TYPE;

typedef enum
{
	EFI_PM_GPIO_INVERT_EXT_PIN_OUTPUT_DISABLE,
	/**< Disabled. */
	EFI_PM_GPIO_INVERT_EXT_PIN_OUTPUT_ENABLE,
	/**< Enabled. */
	EFI_PM_GPIO_INVERT_EXT_PIN_OUTPUT_INVALID
	/**< Invalid external pin output inversion. */
} EFI_PM_GPIO_INVERT_EXT_PIN_TYPE;

typedef enum
{
	EFI_PM_GPIO_OUT_BUFFER_OFF,    /**< OFF. */
	EFI_PM_GPIO_OUT_BUFFER_LOW,    /**< Low. */
	EFI_PM_GPIO_OUT_BUFFER_MEDIUM, /**< Medium. */
	EFI_PM_GPIO_OUT_BUFFER_HIGH,   /**< High. */
	EFI_PM_GPIO_OUT_BUFFER_DRIVE_STRENGTH__INVALID /**< Invalid strength. */
} EFI_PM_GPIO_OUT_BUFFER_DRIVE_STRENGTH_TYPE;

typedef enum
{
	EFI_PM_GPIO_SOURCE_GND,                 /**< Ground. */
	EFI_PM_GPIO_SOURCE_PAIRED_GPIO,         /**< Paired GPIO. */
	EFI_PM_GPIO_SOURCE_SPECIAL_FUNCTION1,   /**< Special function 1. */
	EFI_PM_GPIO_SOURCE_SPECIAL_FUNCTION2,   /**< Special function 2. */
	EFI_PM_GPIO_SOURCE_DTEST1,              /**< DTEST 1. */
	EFI_PM_GPIO_SOURCE_DTEST2,              /**< DTEST 2. */
	EFI_PM_GPIO_SOURCE_DTEST3,              /**< DTEST 3. */
	EFI_PM_GPIO_SOURCE_DTEST4,              /**< DTEST 4. */
	EFI_PM_GPIO_SOURCE_CONFIG_TYPE__INVALID /**< Invalid source configuration. */
} EFI_PM_GPIO_SOURCE_CONFIG_TYPE;

typedef enum
{
	EFI_PM_GPIO_INPUT_ON,
	/**< GPIO is configured as input. */
	EFI_PM_GPIO_INPUT_OUTPUT_ON,
	/**< GPIO is configured as input and output. */
	EFI_PM_GPIO_OUTPUT_ON,
	/**< GPIO is configured as output. */
	EFI_PM_GPIO_INPUT_OUTPUT_OFF
	/**< Both input and output are off. */
} EFI_PM_GPIO_MODE_SELECT_TYPE;

typedef enum
{
	EFI_PM_GPIO_MODE_DISABLE,            /**< Disabled. */
	EFI_PM_GPIO_MODE_ENABLE,             /**< Enabled. */
	EFI_PM_GPIO_MODE_ONOFF_TYPE__INVALID /**< Invalid mode. */
} EFI_PM_GPIO_MODE_ONOFF_TYPE;

typedef enum
{
	EFI_PM_GPIO_DTEST_DISABLE,                   /**< Disabled. */
	EFI_PM_GPIO_DTEST_ENABLE,                    /**< Enabled. */
	EFI_PM_GPIO_DTEST_BUFFER_ONOFF_TYPE__INVALID /**< Invalid buffer. */
} EFI_PM_GPIO_DTEST_BUFFER_ONOFF_TYPE;

typedef enum
{
	EFI_PM_GPIO_EXT_PIN_ENABLE,
	/**< Enabled. */
	EFI_PM_GPIO_EXT_PIN_DISABLE,
	/**< Puts EXT_PIN at a high Z state and disables the block. */
	EFI_PM_GPIO_EXT_PIN_CONFIG_TYPE__INVALID
	/**< Invalid external pin configuration. */
} EFI_PM_GPIO_EXT_PIN_CONFIG_TYPE;

typedef enum
{
	EFI_PM_GPIO_DIG_IN_DTEST1,             /**< DTEST1. */
	EFI_PM_GPIO_DIG_IN_DTEST2,             /**< DTEST2. */
	EFI_PM_GPIO_DIG_IN_DTEST3,             /**< DTEST3. */
	EFI_PM_GPIO_DIG_IN_DTEST4,             /**< DTEST4. */
	EFI_PM_GPIO_DIG_IN_DTEST_TYPE__INVALID /**< Invalid line. */
} EFI_PM_GPIO_DIG_IN_TYPE;

typedef struct
{
	EFI_PM_GPIO_MODE_SELECT_TYPE                GpioModeSelect;
	/**< Output mode; see #EFI_PM_GPIO_MODE_SELECT_TYPE for details. */
	EFI_PM_GPIO_VOLTAGE_SOURCE_TYPE             GpioVoltage_source;
	/**< Voltage source; see #EFI_PM_GPIO_VOLTAGE_SOURCE_TYPE for details. */
	EFI_PM_GPIO_MODE_ONOFF_TYPE                 GpioModeOnOff;
	/**< Enable mode selection; see #EFI_PM_GPIO_MODE_ONOFF_TYPE for
	details. */
	EFI_PM_GPIO_OUT_BUFFER_CONFIG_TYPE          GpioOutBufferConfig;
	/**< Output buffer configuration; see #EFI_PM_GPIO_OUT_BUFFER_CONFIG_TYPE
	for details. */
	EFI_PM_GPIO_INVERT_EXT_PIN_TYPE             GpioInvertExtPin;
	/**< External pin output inversion setting;
	see #EFI_PM_GPIO_INVERT_EXT_PIN_TYPE for details. */
	EFI_PM_GPIO_OUT_BUFFER_DRIVE_STRENGTH_TYPE  GpioOutBufferDriveStrength;
	/**< Output buffer strength;
	see #EFI_PM_GPIO_OUT_BUFFER_DRIVE_STRENGTH_TYPE for details. */
	EFI_PM_GPIO_CURRENT_SOURCE_PULLS_TYPE       GpioCurrentSourcePulls;
	/**< Current source pull; see #EFI_PM_GPIO_CURRENT_SOURCE_PULLS_TYPE
	for details. */
	EFI_PM_GPIO_SOURCE_CONFIG_TYPE              GpioSourceConfig;
	/**< Source configuration; see #EFI_PM_GPIO_SOURCE_CONFIG_TYPE for
	details. */
	EFI_PM_GPIO_DTEST_BUFFER_ONOFF_TYPE         GpioDtestBufferOnOff;
	/**< Enable DTEST buffer; see #EFI_PM_GPIO_DTEST_BUFFER_ONOFF_TYPE for
	details. */
	EFI_PM_GPIO_EXT_PIN_CONFIG_TYPE             GpioExtPinConfig;
	/**< Enable external pin configuration;
	see #EFI_PM_GPIO_EXT_PIN_CONFIG_TYPE for details. */
} EFI_PM_GPIO_STATUS_TYPE;

typedef
EFI_STATUS(EFIAPI *EFI_PM_GPIO_CONFIG_DIGITAL_INPUT)(
	IN UINT32                                     PmicDeviceIndex,
	IN EFI_PM_GPIO_WHICH_TYPE                     Gpio,
	IN EFI_PM_GPIO_CURRENT_SOURCE_PULLS_TYPE      ISourcePulls,
	IN EFI_PM_GPIO_VOLTAGE_SOURCE_TYPE            VoltageSource,
	IN EFI_PM_GPIO_OUT_BUFFER_DRIVE_STRENGTH_TYPE OutBufferStrength,
	IN EFI_PM_GPIO_SOURCE_CONFIG_TYPE             Source
);

typedef
EFI_STATUS(EFIAPI *EFI_PM_GPIO_CONFIG_DIGITAL_OUTPUT)(
	IN UINT32                                     PmicDeviceIndex,
	IN EFI_PM_GPIO_WHICH_TYPE                     Gpio,
	IN EFI_PM_GPIO_OUT_BUFFER_CONFIG_TYPE         OutBufferConfig,
	IN EFI_PM_GPIO_VOLTAGE_SOURCE_TYPE            VoltageSource,
	IN EFI_PM_GPIO_SOURCE_CONFIG_TYPE             Source,
	IN EFI_PM_GPIO_OUT_BUFFER_DRIVE_STRENGTH_TYPE OutBufferStrength,
	IN BOOLEAN                                    OutInversion
);

struct _EFI_QCOM_PMIC_GPIO_SHIM_PROTOCOL {
	UINT64                                Revision;
	EFI_PM_GPIO_CONFIG_DIGITAL_INPUT      ConfigDigitalInput;
	EFI_PM_GPIO_CONFIG_DIGITAL_OUTPUT     ConfigDigitalOutput;
	// Not support the rest
};

#endif