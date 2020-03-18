#ifndef _SYNAPTICS_RMI4_H_
#define _SYNAPTICS_RMI4_H_

#define TOUCH_RMI_PAGE_INFO_BYTES 6
#define TOUCH_RMI_PAGE_INFO_ADDRESS 0xE9
#define TOUCH_RMI_MAX_FUNCTIONS 10
#define TOUCH_RMI_F12_FUNCTION 0x12

#define TOUCH_DELAY_TO_COMMUNICATE 200000
#define TOUCH_POWER_RAIL_STABLE_TIME 2000

#define RMI_CHANGE_PAGE_ADDRESS 0xFF

#define TIMER_INTERVAL_TOUCH_POLL 100000
#define TOUCH_BUFFER_SIZE 25
#define TOUCH_DATA_BYTES 8

#define FAILURE_THRESHOLD 25

typedef struct _TOUCH_DATA {
  UINT16 TouchX;
  UINT16 TouchY;
  UINT8  TouchStatus;
} TOUCH_DATA, *PTOUCH_DATA;

typedef struct _RMI4_INTERNAL_DATA {
  UINT32                        Signature;
  UINT16                        LastX;
  UINT16                        LastY;
  EFI_ABSOLUTE_POINTER_PROTOCOL AbsPointerProtocol;
  EFI_ABSOLUTE_POINTER_MODE     AbsPointerMode;
  EFI_EVENT                     PollingTimerEvent;
  BOOLEAN                       Initialized;
  UINT8                         PageF12;
  UINT32                        TouchDataAddress;
  UINT32                        FailureCount;
  QCOM_GPIO_TLMM_PROTOCOL *     GpioTlmmProtocol;
  QCOM_I2C_QUP_PROTOCOL *       I2cQupProtocol;
  struct qup_i2c_dev *          I2cController;
} RMI4_INTERNAL_DATA;

#define RMI4_TCH_INSTANCE_SIGNATURE SIGNATURE_32('r', 'm', 'i', '4')
#define RMI4_TCH_INSTANCE_FROM_BLOCKIO_THIS(a)                                 \
  CR(a, RMI4_INTERNAL_DATA, AbsPointerProtocol, RMI4_TCH_INSTANCE_SIGNATURE)

EFI_STATUS
EFIAPI
SynaI2cRead(
    RMI4_INTERNAL_DATA *Instance, IN UINT8 Address, IN UINT8 *Data,
    IN UINT16 ReadBytes);

EFI_STATUS
EFIAPI
SynaI2cWrite(
    RMI4_INTERNAL_DATA *Instance, IN UINT8 Address, IN UINT8 *Data,
    IN UINT16 WriteBytes);

EFI_STATUS AbsPGetState(
    IN EFI_ABSOLUTE_POINTER_PROTOCOL *This,
    IN OUT EFI_ABSOLUTE_POINTER_STATE *State);

EFI_STATUS AbsPReset(
    IN EFI_ABSOLUTE_POINTER_PROTOCOL *This, IN BOOLEAN ExtendedVerification);

EFI_STATUS AbsStartPolling(IN RMI4_INTERNAL_DATA *Instance);

VOID EFIAPI SyncPollCallback(IN EFI_EVENT Event, IN VOID *Context);

EFI_STATUS EFIAPI SynaPowerUpController(RMI4_INTERNAL_DATA *Instance);

#endif