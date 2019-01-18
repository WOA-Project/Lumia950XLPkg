// Clock Shim
// This is intended to maintain compatibility with Qualcomm binary module.
// Do not invoke if not necessary.

#ifndef __CLOCK_SHIM_H__
#define __CLOCK_SHIM_H__

#define EFI_CLOCK_SHIM_PROTOCOL_VERSION 0x0000000000010005
#define EFI_CLOCK_SHIM_PROTOCOL_GUID \
  { 0x241AFAE6, 0x885F, 0x4F6C, {0xA7, 0xEA, 0xC2, 0x8E, 0xAB, 0x79, 0xC3, 0xE5 } }

extern EFI_GUID gEfiClockShimProtocolGuid;

typedef struct _EFI_CLOCK_SHIM_PROTOCOL EFI_CLOCK_SHIM_PROTOCOL;

typedef enum
{
	EFI_CLOCK_FREQUENCY_HZ_AT_LEAST = 0,
	EFI_CLOCK_FREQUENCY_HZ_AT_MOST = 1,
	EFI_CLOCK_FREQUENCY_HZ_CLOSEST = 2,
	EFI_CLOCK_FREQUENCY_HZ_EXACT = 3,
	EFI_CLOCK_FREQUENCY_KHZ_AT_LEAST = 0x10,
	EFI_CLOCK_FREQUENCY_KHZ_AT_MOST = 0x11,
	EFI_CLOCK_FREQUENCY_KHZ_CLOSEST = 0x12,
	EFI_CLOCK_FREQUENCY_KHZ_EXACT = 0x13,
	EFI_CLOCK_FREQUENCY_MHZ_AT_LEAST = 0x20,
	EFI_CLOCK_FREQUENCY_MHZ_AT_MOST = 0x21,
	EFI_CLOCK_FREQUENCY_MHZ_CLOSEST = 0x22,
	EFI_CLOCK_FREQUENCY_MHZ_EXACT = 0x23,
} EFI_CLOCK_FREQUENCY_TYPE;

typedef enum
{
	EFI_CLOCK_RESET_DEASSERT = 0,
	EFI_CLOCK_RESET_ASSERT = 1,
	EFI_CLOCK_RESET_PULSE = 2
} EFI_CLOCK_RESET_TYPE;

typedef enum {

	EFI_CLOCK_LPM_LEVEL1 = 1,
	EFI_CLOCK_LPM_LEVEL2 = 2,
	EFI_CLOCK_LPM_LEVEL3 = 3,
} EFI_CLOCK_LPM_TYPE;

typedef
EFI_STATUS
(EFIAPI *EFI_CLOCK_GET_ID)(
	IN  EFI_CLOCK_SHIM_PROTOCOL *This,
	IN  CONST CHAR8        *ClockName,
	OUT UINTN              *ClockId
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOCK_ENABLE)(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN UINTN              ClockId
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOCK_DISABLE)(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN UINTN              ClockId
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOCK_IS_ENABLED)(
	IN  EFI_CLOCK_SHIM_PROTOCOL *This,
	IN  UINTN              ClockId,
	OUT BOOLEAN            *IsEnabled
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOCK_IS_ON)(
	IN  EFI_CLOCK_SHIM_PROTOCOL *This,
	IN  UINTN              ClockId,
	OUT BOOLEAN            *IsOn
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOCK_SET_FREQ_HZ)(
	IN  EFI_CLOCK_SHIM_PROTOCOL        *This,
	IN  UINTN						ClockId,
	IN  UINT32                    Freq,
	IN  EFI_CLOCK_FREQUENCY_TYPE  Match,
	OUT UINT32                    *ResultFreq
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOCK_GET_FREQ_HZ)(
	IN  EFI_CLOCK_SHIM_PROTOCOL *This,
	IN  UINTN              ClockId,
	OUT UINT32             *FreqHz
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOCK_CALC_FREQ_HZ)(
	IN  EFI_CLOCK_SHIM_PROTOCOL *This,
	IN  UINTN              ClockId,
	OUT UINT32             *FreqHz
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOCK_SELECT_EXTERNAL_SOURCE)(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN UINTN              ClockId,
	IN UINT32             FreqHz,
	IN UINT32             Source,
	IN UINT32             Divider,
	IN UINT32             VAL_M,
	IN UINT32             VAL_N,
	IN UINT32             VAL_2D
);

typedef
EFI_STATUS
(EFIAPI *EFI_POWER_DOMAIN_GET_ID)(
	IN  EFI_CLOCK_SHIM_PROTOCOL *This,
	IN  CONST CHAR8        *ClockPowerDomainName,
	OUT UINTN              *ClockPowerDomainId
);

typedef
EFI_STATUS
(EFIAPI *EFI_POWER_DOMAIN_ENABLE)(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN UINTN              ClockPowerDomainId
);

typedef
EFI_STATUS
(EFIAPI *EFI_POWER_DOMAIN_DISABLE)(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN UINTN              ClockPowerDomainId
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOCK_ENTER_LOW_POWER_MODE)(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN EFI_CLOCK_LPM_TYPE lpmtype
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOCK_EXIT_LOW_POWER_MODE)(
	IN EFI_CLOCK_SHIM_PROTOCOL *This,
	IN EFI_CLOCK_LPM_TYPE lpmtype
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOCK_SET_CLOCK_DIVIDER)(
	IN  EFI_CLOCK_SHIM_PROTOCOL        *This,
	IN  UINTN                     ClockId,
	IN  UINT32                    Divider
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOCK_RESET)(
	IN EFI_CLOCK_SHIM_PROTOCOL    *This,
	IN UINTN                  ClockId,
	IN EFI_CLOCK_RESET_TYPE   eReset
);

struct _EFI_CLOCK_SHIM_PROTOCOL {
	UINT64                           Version;
	EFI_CLOCK_GET_ID                 GetClockID;
	EFI_CLOCK_ENABLE                 EnableClock;
	EFI_CLOCK_DISABLE                DisableClock;
	EFI_CLOCK_IS_ENABLED             IsClockEnabled;
	EFI_CLOCK_IS_ON                  IsClockOn;
	EFI_CLOCK_SET_FREQ_HZ            SetClockFreqHz;
	EFI_CLOCK_GET_FREQ_HZ            GetClockFreqHz;
	EFI_CLOCK_CALC_FREQ_HZ           CalcClockFreqHz;
	EFI_CLOCK_SELECT_EXTERNAL_SOURCE SelectExternalSource;
	EFI_POWER_DOMAIN_GET_ID          GetClockPowerDomainID;
	EFI_POWER_DOMAIN_ENABLE          EnableClockPowerDomain;
	EFI_POWER_DOMAIN_DISABLE         DisableClockPowerDomain;
	EFI_CLOCK_ENTER_LOW_POWER_MODE   EnterLowPowerMode;
	EFI_CLOCK_EXIT_LOW_POWER_MODE    ExitLowPowerMode;
	EFI_CLOCK_SET_CLOCK_DIVIDER      SetClockDivider;
	EFI_CLOCK_RESET                  ResetClock;
};

#endif