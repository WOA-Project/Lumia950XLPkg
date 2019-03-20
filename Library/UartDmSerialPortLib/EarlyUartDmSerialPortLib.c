#include <PiPei.h>

#include <Library/LKEnvLib.h>

#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/SerialPortLib.h>

#include <Library/QcomTargetUartDmLib.h>

#include "uartdm_p.h"

RETURN_STATUS
EFIAPI
SerialPortInitialize(VOID)
{
  RETURN_STATUS Status;
  UINT8         Id;
  UINTN         GsbiBase;
  UINTN         UartDmBase;

  // call library constructors
  LibQcomTargetUartCallSecConstructors();

  // get UART config
  Status = LibQcomTargetGetUartDmConfig(&Id, &GsbiBase, &UartDmBase);
  if (RETURN_ERROR(Status)) {
    return Status;
  }

  // initialize UART
  uart_dm_init(Id, GsbiBase, UartDmBase);
  g_uart_dm_base = UartDmBase;

  // store UARTDM base in a Hob
  UINT64 *UartHobData;
  UartHobData = BuildGuidHob(&gQcomUartDmBaseGuid, sizeof *UartHobData);
  ASSERT(UartHobData != NULL);
  *UartHobData = UartDmBase;

  return Status;
}

/**
  Write data to serial device.

  @param  Buffer           Point of data buffer which need to be written.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Write data failed.
  @retval !0               Actual number of bytes written to serial device.

**/
UINTN
EFIAPI
SerialPortWrite(IN UINT8 *Buffer, IN UINTN NumberOfBytes)
{
  UINTN  Num         = 0;
  UINT8 *CONST Final = &Buffer[NumberOfBytes];
  while (Buffer < Final) {
    int rc = uart_putc(*Buffer++);
    if (rc <= 0) {
      break;
    }
    else {
      Num += rc;
    }
  }
  return Num;
}

/**
  Read data from serial device and save the data in buffer.

  @param  Buffer           Point of data buffer which need to be written.
  @param  NumberOfBytes    Size of Buffer[].

  @retval 0                Read data failed.
  @retval !0               Actual number of bytes read from serial device.

**/
UINTN
EFIAPI
SerialPortRead(OUT UINT8 *Buffer, IN UINTN NumberOfBytes)
{
  UINTN  Num         = 0;
  UINT8 *CONST Final = &Buffer[NumberOfBytes];
  while (Buffer < Final) {
    int rc = uart_getc(Buffer++, TRUE);
    if (rc <= 0) {
      break;
    }
    else {
      Num += rc;
    }
  }
  return Num;
}

/**
  Check to see if any data is available to be read from the debug device.

  @retval TRUE       At least one byte of data is available to be read
  @retval FALSE      No data is available to be read

**/
BOOLEAN
EFIAPI
SerialPortPoll(VOID) { return uart_tstc() == 1; }

/**
  Sets the control bits on a serial device.

  @param[in] Control            Sets the bits of Control that are settable.

  @retval RETURN_SUCCESS        The new control bits were set on the serial
device.
  @retval RETURN_UNSUPPORTED    The serial device does not support this
operation.
  @retval RETURN_DEVICE_ERROR   The serial device is not functioning correctly.

**/
RETURN_STATUS
EFIAPI
SerialPortSetControl(IN UINT32 Control) { return RETURN_UNSUPPORTED; }

/**
  Retrieve the status of the control bits on a serial device.

  @param[out] Control           A pointer to return the current control signals
from the serial device.

  @retval RETURN_SUCCESS        The control bits were read from the serial
device.
  @retval RETURN_UNSUPPORTED    The serial device does not support this
operation.
  @retval RETURN_DEVICE_ERROR   The serial device is not functioning correctly.

**/
RETURN_STATUS
EFIAPI
SerialPortGetControl(OUT UINT32 *Control) { return RETURN_UNSUPPORTED; }

/**
  Sets the baud rate, receive FIFO depth, transmit/receice time out, parity,
  data bits, and stop bits on a serial device.

  @param BaudRate           The requested baud rate. A BaudRate value of 0 will
use the device's default interface speed. On output, the value actually set.
  @param ReveiveFifoDepth   The requested depth of the FIFO on the receive side
of the serial interface. A ReceiveFifoDepth value of 0 will use the device's
default FIFO depth. On output, the value actually set.
  @param Timeout            The requested time out for a single character in
microseconds. This timeout applies to both the transmit and receive side of the
                            interface. A Timeout value of 0 will use the
device's default time out value. On output, the value actually set.
  @param Parity             The type of parity to use on this serial device. A
Parity value of DefaultParity will use the device's default parity value. On
output, the value actually set.
  @param DataBits           The number of data bits to use on the serial device.
A DataBits vaule of 0 will use the device's default data bit setting. On output,
the value actually set.
  @param StopBits           The number of stop bits to use on this serial
device. A StopBits value of DefaultStopBits will use the device's default number
of stop bits. On output, the value actually set.

  @retval RETURN_SUCCESS            The new attributes were set on the serial
device.
  @retval RETURN_UNSUPPORTED        The serial device does not support this
operation.
  @retval RETURN_INVALID_PARAMETER  One or more of the attributes has an
unsupported value.
  @retval RETURN_DEVICE_ERROR       The serial device is not functioning
correctly.

**/
RETURN_STATUS
EFIAPI
SerialPortSetAttributes(
    IN OUT UINT64 *BaudRate, IN OUT UINT32 *ReceiveFifoDepth,
    IN OUT UINT32 *Timeout, IN OUT EFI_PARITY_TYPE *Parity,
    IN OUT UINT8 *DataBits, IN OUT EFI_STOP_BITS_TYPE *StopBits)
{
  return RETURN_UNSUPPORTED;
}
