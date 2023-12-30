/** @file
  Uart Serial Port library instance with empty functions.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    sdm845v2.c

Abstract:

    This module contains support for the Qualcomm SDM 845 serial UART with
    adjusted baud rate divider settings.

--*/

#include <Base.h>
#include <Uefi.h>
#include <Uefi/UefiBaseType.h>
#include <Library/SerialPortLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include "sdm845.h"

//
// This is the control transfer data block.
//

static FIFO_TX_BLOCK Transfer;

BOOLEAN
SDM845v2SetBaud (
    EFI_PHYSICAL_ADDRESS Address,
    UINT32 Rate
    )

/*++

Routine Description:

    Set the baud rate for the UART hardware and record it in the port object.

Arguments:

    Port - Supplies the address of the port object that describes the UART.

    Rate - Supplies the desired baud rate in bits per second.

Return Value:

    TRUE if the baud rate was programmed, FALSE if it was not.

--*/

{

    UINT32 DivisorLatch = 0;
    UINT32 ValueTempMask = 0;

    if ((Address == 0)) {
        return FALSE;
    }

    DivisorLatch = UART_DM_READ_REG(Address + GENI_CFG,
                                    HWIO_GENI_SER_M_CLK_CFG_OFFS);

    if (DivisorLatch == 0x11) {
        return FALSE;
    }

    //
    // Clock rate is 7.372 mhz.
    //

    switch (Rate) {
    case 7200:
        DivisorLatch = 0x40;
        break;
    case 9600:
        DivisorLatch = 0x30;
        break;
    case 14400:
        DivisorLatch = 0x20;
        break;
    case 19200:
        DivisorLatch = 0x18;
        break;
    case 28800:
        DivisorLatch = 0x10;
        break;
    case 38400:
        DivisorLatch = 0xC;
        break;
    case 57600:
        DivisorLatch = 0x8;
        break;
    case 115200:
        DivisorLatch = 0x4;
        break;
    default:
        DivisorLatch = 0x1;
        break;
    }

    ValueTempMask = (DivisorLatch << (HWIO_GENI_SER_M_CLK_CFG_CLK_DIV_VALUE_SHFT)) |
                     HWIO_GENI_SER_M_CLK_CFG_SER_CLK_EN_BMSK;

    UART_DM_WRITE_REG(Address + GENI_CFG,
                      HWIO_GENI_SER_M_CLK_CFG_OFFS,
                      ValueTempMask);

    UART_DM_WRITE_REG(Address + GENI_CFG,
                      HWIO_GENI_SER_S_CLK_CFG_OFFS,
                      ValueTempMask);

    return TRUE;
}


BOOLEAN
SDM845v2InitializePort (
    EFI_PHYSICAL_ADDRESS Address,
    UINT32 BaudRate
    )

/*++

Routine Description:

    This routine performs the re-initialization of an SDM845 serial UART.

Arguments:

    Port - Supplies a pointer to a CPPORT object which will be filled in as
        part of the port initialization.

Return Value:

    TRUE if the port has been successfully initialized, FALSE otherwise.

--*/

{

    UINT32 ConfigMask;
    UINT32 Retries;

    //
    // Wait for the state machine to clear up.
    //

    Retries = 0;
    while ((UART_DM_READ_REG(Address + GENI_CFG, HWIO_GENI_STATUS_OFFS) &
            HWIO_GENI_STATUS_M_GENI_CMD_ACTIVE_BMSK)) {

        Retries += 1;
        if (Retries >= MAX_RETRIES) {
            return FALSE;
        }
    }

    UART_DM_WRITE_REG(Address + GENI_CFG, HWIO_GENI_DFS_IF_CFG_OFFS, 0x0);

    ConfigMask = (HWIO_DMA_GENERAL_CFG_AHB_SEC_SLV_CLK_CGC_ON_BMSK << HWIO_DMA_GENERAL_CFG_AHB_SEC_SLV_CLK_CGC_ON_SHFT) |
                 (HWIO_DMA_GENERAL_CFG_DMA_AHB_SLV_CLK_CGC_ON_BMSK << HWIO_DMA_GENERAL_CFG_DMA_AHB_SLV_CLK_CGC_ON_SHFT) |
                 (HWIO_DMA_GENERAL_CFG_DMA_TX_CLK_CGC_ON_BMSK << HWIO_DMA_GENERAL_CFG_DMA_TX_CLK_CGC_ON_SHFT) |
                 (HWIO_DMA_GENERAL_CFG_DMA_RX_CLK_CGC_ON_BMSK << HWIO_DMA_GENERAL_CFG_DMA_RX_CLK_CGC_ON_SHFT);

    UART_DM_WRITE_REG(Address + QUPV3_SE_DMA, HWIO_DMA_GENERAL_CFG_OFFS, ConfigMask);
    UART_DM_WRITE_REG(Address + GENI_CFG, HWIO_GENI_CGC_CTRL_OFFS, 0x7F);
    UART_DM_WRITE_REG(Address + GENI_CFG, HWIO_GENI_FORCE_DEFAULT_REG_OFFS, 0x1);
    UART_DM_WRITE_REG(Address + GENI_CFG, HWIO_GENI_OUTPUT_CTRL_OFFS, 0x7F);
    UART_DM_WRITE_REG(Address + GENI_IMAGE_REGS, HWIO_GENI_DMA_MODE_EN_OFFS, 0);
    UART_DM_WRITE_REG(Address + QUPV3_SE_DMA, HWIO_SE_IRQ_EN_OFFS, 0xFFFFFFFF);
    UART_DM_WRITE_REG(Address + QUPV3_SE_DMA, HWIO_SE_GSI_EVENT_EN_OFFS, 0);
    UART_DM_WRITE_REG(Address + GENI_DATA, HWIO_GENI_M_IRQ_ENABLE_OFFS, 0xB300005F);
    UART_DM_WRITE_REG(Address + GENI_DATA, HWIO_GENI_S_IRQ_ENABLE_OFFS, 0xB3007E5F);

    //
    // Get TX FIFO width.
    //

    ConfigMask = UART_DM_READ_REG(Address + QUPV3_SE_DMA, HWIO_SE_HW_PARAM_0_OFFS);
    ConfigMask = (ConfigMask & TX_FIFO_DEPTH_MASK) >> TX_FIFO_DEPTH_SHIFT;
    UART_DM_WRITE_REG(Address + GENI_DATA, HWIO_GENI_TX_WATERMARK_REG_OFFS, 4);

    //
    // Get RX FIFO width.
    //

    ConfigMask = UART_DM_READ_REG(Address + QUPV3_SE_DMA, HWIO_SE_HW_PARAM_1_OFFS);
    ConfigMask = (ConfigMask & RX_FIFO_DEPTH_MASK) >> RX_FIFO_DEPTH_SHIFT;
    UART_DM_WRITE_REG(Address + GENI_DATA, HWIO_GENI_RX_WATERMARK_REG_OFFS, (ConfigMask - 8));
    UART_DM_WRITE_REG(Address + GENI_DATA, HWIO_GENI_RX_RFR_WATERMARK_REG_OFFS, (ConfigMask - 4));

    SDM845v2SetBaud(Address, BaudRate);

    UART_DM_WRITE_REG(Address + GENI_IMAGE_REGS, HWIO_UART_TX_WORD_LEN_OFFS, 0x8);
    UART_DM_WRITE_REG(Address + GENI_IMAGE_REGS, HWIO_UART_RX_WORD_LEN_OFFS, 0x8);
    UART_DM_WRITE_REG(Address + GENI_IMAGE_REGS, HWIO_UART_TX_PARITY_CFG_OFFS, 0x0);
    UART_DM_WRITE_REG(Address + GENI_IMAGE_REGS, HWIO_UART_TX_TRANS_CFG_OFFS, 0x2);
    UART_DM_WRITE_REG(Address + GENI_IMAGE_REGS, HWIO_UART_RX_PARITY_CFG_OFFS, 0x0);
    UART_DM_WRITE_REG(Address + GENI_IMAGE_REGS, HWIO_UART_RX_TRANS_CFG_OFFS, 0x0);
    UART_DM_WRITE_REG(Address + GENI_IMAGE_REGS, HWIO_UART_TX_STOP_BIT_LEN_OFFS, 0x0);
    UART_DM_WRITE_REG(Address + GENI_IMAGE_REGS, HWIO_UART_RX_STALE_CNT_OFFS, (0x16 * 10));
    UART_DM_WRITE_REG(Address + GENI_DATA, HWIO_GENI_S_CMD0_OFFS, 0x8000000);
    return TRUE;
}

EFI_STATUS
SDM845GetByte (
    EFI_PHYSICAL_ADDRESS Address,
    UINT32 BaudRate,
    UINT8* Byte
    )

/*++

Routine Description:

    Fetch a data byte from the UART device and return it.

Arguments:

    Port - Supplies the address of the port object that describes the UART.

    Byte - Supplies the address of variable to hold the result.

Return Value:

    EFI_STATUS code.

--*/

{

    UINT32 AvailableBytes;
    UINT8 BaseAddress;
    UINT32 IrqStatus;
    UINT32 RxFifoStatus;
    UINT32 PartialBytesToRead;
    UINT32 WordsToRead;
    UINT32 ArrayIndex;
    UINT32 Index;
    UINT32 RxFifo;

    if ((Address == 0)) {
        return EFI_NOT_READY;
    }

    BaseAddress = Address;
    ArrayIndex = 0;

    //
    // Check if there is any available data. If not, then read from the FIFO
    // and fill the local transfer buffer.
    //

    if (Transfer.AvailableBytes == 0) {
        Transfer.PtrToFifoBuffer = (UINT8 *)Transfer.FifoBuffer;
        IrqStatus = UART_DM_READ_REG(BaseAddress + GENI_DATA, HWIO_GENI_S_IRQ_STATUS_OFFS);
        UART_DM_WRITE_REG(BaseAddress + GENI_DATA, HWIO_GENI_S_IRQ_CLEAR_OFFS, IrqStatus);
        RxFifoStatus = UART_DM_READ_REG(BaseAddress + GENI_DATA, HWIO_GENI_RX_FIFO_STATUS_OFFS);

        //
        // Calculate the number of bytes to read.
        //

        PartialBytesToRead = (RxFifoStatus & RX_LAST_VALID_BYTES_MASK) >> RX_LAST_VALID_BYTES_SHIFT;
        WordsToRead = RxFifoStatus & RX_FIFO_WC;
        if ((PartialBytesToRead > 0) || (WordsToRead > 0)) {
            if ((PartialBytesToRead != 0) && (PartialBytesToRead != 4)) {
                WordsToRead -= 1;
            }

        } else if ((IrqStatus & RX_FIFO_WATERMARK_IRQ) != 0) {
            WordsToRead = UART_DM_READ_REG(BaseAddress + GENI_DATA, HWIO_GENI_RX_WATERMARK_REG_OFFS);
        }

        //
        // Don't overrun the local transfer buffer.
        //

        AvailableBytes = (WordsToRead * 4) + PartialBytesToRead;
        if (AvailableBytes > MAX_RX_FIFO_SIZE) {
            SDM845v2InitializePort(Address, BaudRate);
            return EFI_NOT_READY;
        }

        Transfer.AvailableBytes = AvailableBytes;
        for (Index = 0; Index < WordsToRead; Index += 1) {
            RxFifo = UART_DM_READ_REG(BaseAddress + GENI_DATA, HWIO_GENI_RX_FIFOn_OFFS(BaseAddress, Index));
            Transfer.FifoBuffer[0 + ArrayIndex] = (UINT8)(RxFifo >>  0);
            Transfer.FifoBuffer[1 + ArrayIndex] = (UINT8)(RxFifo >>  8);
            Transfer.FifoBuffer[2 + ArrayIndex] = (UINT8)(RxFifo >> 16);
            Transfer.FifoBuffer[3 + ArrayIndex] = (UINT8)(RxFifo >> 24);
            ArrayIndex += 4;
        }

        if (PartialBytesToRead > 0) {
            RxFifo = UART_DM_READ_REG(BaseAddress + GENI_DATA, HWIO_GENI_RX_FIFOn_OFFS(BaseAddress, Index));
            for (Index = 0; Index < PartialBytesToRead; Index += 1) {
                Transfer.FifoBuffer[ArrayIndex] = (UINT8)(RxFifo >> Index * 8);
                ArrayIndex += 1;
            }
        }
    }

    if (Transfer.AvailableBytes != 0) {
        *Byte = *Transfer.PtrToFifoBuffer;
        Transfer.PtrToFifoBuffer += 1;
        Transfer.AvailableBytes -= 1;
        return EFI_SUCCESS;
    }

    return EFI_OUT_OF_RESOURCES;
}

EFI_STATUS
SDM845PutByte (
    EFI_PHYSICAL_ADDRESS Address,
    UINT8 Byte,
    BOOLEAN BusyWait
    )

/*++

Routine Description:

    Write a data byte out to the UART device.

Arguments:

    Port - Supplies the address of the port object that describes the UART.

    Byte - Supplies the data to emit.

    BusyWait - Supplies a flag to control whether this routine will busy
        wait (spin) for the UART hardware to be ready to transmit.

Return Value:

    EFI_STATUS code.

--*/

{
    if ((Address == 0)) {
        return EFI_NOT_READY;
    }

    //
    // Wait for the state machine to clear up.
    //

    if (BusyWait != FALSE) {
        while((UART_DM_READ_REG(Address + GENI_CFG, HWIO_GENI_STATUS_OFFS) & HWIO_GENI_STATUS_M_GENI_CMD_ACTIVE_BMSK));

    } else if ((UART_DM_READ_REG(Address + GENI_CFG, HWIO_GENI_STATUS_OFFS) & HWIO_GENI_STATUS_M_GENI_CMD_ACTIVE_BMSK)) {

        return EFI_NOT_READY;
    }

    UART_DM_WRITE_REG(Address + GENI_IMAGE_REGS, HWIO_UART_TX_TRANS_LEN_OFFS, 1);
    UART_DM_WRITE_REG(Address + GENI_DATA, HWIO_GENI_M_CMD0_OFFS, 0x8000000);

    UINT32 word_value = 0;
    word_value = word_value | Byte;
    UART_DM_WRITE_REG(Address + GENI_DATA, HWIO_GENI_TX_FIFOn_OFFS(Address, 0), word_value);
    return EFI_SUCCESS;
}

BOOLEAN
SDM845RxReady (
    EFI_PHYSICAL_ADDRESS Address
    )

/*++

Routine Description:

    This routine determines if there is data pending in the UART.

Arguments:

    Port - Supplies the address of the port object that describes the UART.

Return Value:

    TRUE if data is available, FALSE otherwise.

--*/

{

    UINT8 BaseAddress;
    UINT32 FifoStatReg;
    UINT32 PartialBytesToRead;
    UINT32 WordsToRead;
    BOOLEAN IsAvailableBytes;
    UINT32 IrqStatus;

    if ((Address == 0)) {
        return FALSE;
    }

    //
    // Check if there is any waiting pending data.
    //

    if (Transfer.AvailableBytes != 0) {
        IsAvailableBytes = TRUE;
        goto SDM845ReceiveDataAvailableEnd;
    }

    //
    // Read the FIFO status register
    //

    BaseAddress = Address;
    FifoStatReg = UART_DM_READ_REG(BaseAddress + GENI_DATA, HWIO_GENI_RX_FIFO_STATUS_OFFS);

    //
    // Calculate the number of bytes to read
    //

    PartialBytesToRead = (FifoStatReg & RX_LAST_VALID_BYTES_MASK) >> RX_LAST_VALID_BYTES_SHIFT;
    WordsToRead = FifoStatReg & RX_FIFO_WC;
    IsAvailableBytes = FALSE;
    if ((PartialBytesToRead > 0) || (WordsToRead > 0)) {
        IsAvailableBytes = TRUE;

    } else {
        IrqStatus = UART_DM_READ_REG(BaseAddress + GENI_DATA, HWIO_GENI_S_IRQ_STATUS_OFFS);
        if ((IrqStatus & RX_FIFO_WATERMARK_IRQ) != 0) {
            if (UART_DM_READ_REG(BaseAddress + GENI_DATA, HWIO_GENI_RX_WATERMARK_REG_OFFS) > 0) {
                IsAvailableBytes = TRUE;
            }
        }
    }

SDM845ReceiveDataAvailableEnd:
    return IsAvailableBytes;
}

// -------------------------------------------------------------------- Globals

/**
  Initialize the serial device hardware.

  If no initialization is required, then return RETURN_SUCCESS.
  If the serial device was successfully initialized, then return RETURN_SUCCESS.
  If the serial device could not be initialized, then return RETURN_DEVICE_ERROR.

  @retval RETURN_SUCCESS        The serial device was initialized.
  @retval RETURN_DEVICE_ERROR   The serial device could not be initialized.

**/
RETURN_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
{
  return SDM845v2InitializePort(FixedPcdGet32(PcdUartSerialBase), FixedPcdGet32(PcdUartDefaultBaudRate));
}

/**
  Write data from buffer to serial device.

  Writes NumberOfBytes data bytes from Buffer to the serial device.
  The number of bytes actually written to the serial device is returned.
  If the return value is less than NumberOfBytes, then the write operation failed.
  If Buffer is NULL, then ASSERT().
  If NumberOfBytes is zero, then return 0.

  @param  Buffer           The pointer to the data buffer to be written.
  @param  NumberOfBytes    The number of bytes to written to the serial device.

  @retval 0                NumberOfBytes is 0.
  @retval >0               The number of bytes written to the serial device.
                           If this value is less than NumberOfBytes, then the write operation failed.

**/
UINTN
EFIAPI
SerialPortWrite (
  IN UINT8  *Buffer,
  IN UINTN  NumberOfBytes
  )
{
  UINTN i;

  for (i = 0; i < NumberOfBytes; i++)
  {
    if (EFI_ERROR(SDM845PutByte(FixedPcdGet32(PcdUartSerialBase), Buffer[i], TRUE)))
    {
      break;
    }
  }

  return i;
}

/**
  Read data from serial device and save the datas in buffer.

  Reads NumberOfBytes data bytes from a serial device into the buffer
  specified by Buffer. The number of bytes actually read is returned.
  If the return value is less than NumberOfBytes, then the rest operation failed.
  If Buffer is NULL, then ASSERT().
  If NumberOfBytes is zero, then return 0.

  @param  Buffer           The pointer to the data buffer to store the data read from the serial device.
  @param  NumberOfBytes    The number of bytes which will be read.

  @retval 0                Read data failed; No data is to be read.
  @retval >0               The actual number of bytes read from serial device.

**/
UINTN
EFIAPI
SerialPortRead (
  OUT UINT8  *Buffer,
  IN  UINTN  NumberOfBytes
  )
{
  UINTN i;

  for (i = 0; i < NumberOfBytes; i++)
  {
    if (EFI_ERROR(SDM845GetByte(FixedPcdGet32(PcdUartSerialBase), FixedPcdGet32(PcdUartDefaultBaudRate), Buffer + i)))
    {
      break;
    }
  }

  return i;
}

/**
  Polls a serial device to see if there is any data waiting to be read.

  Polls a serial device to see if there is any data waiting to be read.
  If there is data waiting to be read from the serial device, then TRUE is returned.
  If there is no data waiting to be read from the serial device, then FALSE is returned.

  @retval TRUE             Data is waiting to be read from the serial device.
  @retval FALSE            There is no data waiting to be read from the serial device.

**/
BOOLEAN
EFIAPI
SerialPortPoll (
  VOID
  )
{
  return SDM845RxReady(FixedPcdGet32(PcdUartSerialBase));
}

/**
  Sets the control bits on a serial device.

  @param Control                Sets the bits of Control that are settable.

  @retval RETURN_SUCCESS        The new control bits were set on the serial device.
  @retval RETURN_UNSUPPORTED    The serial device does not support this operation.
  @retval RETURN_DEVICE_ERROR   The serial device is not functioning correctly.

**/
RETURN_STATUS
EFIAPI
SerialPortSetControl (
  IN UINT32  Control
  )
{
  return RETURN_UNSUPPORTED;
}

/**
  Retrieve the status of the control bits on a serial device.

  @param Control                A pointer to return the current control signals from the serial device.

  @retval RETURN_SUCCESS        The control bits were read from the serial device.
  @retval RETURN_UNSUPPORTED    The serial device does not support this operation.
  @retval RETURN_DEVICE_ERROR   The serial device is not functioning correctly.

**/
RETURN_STATUS
EFIAPI
SerialPortGetControl (
  OUT UINT32  *Control
  )
{
  return RETURN_UNSUPPORTED;
}

/**
  Sets the baud rate, receive FIFO depth, transmit/receice time out, parity,
  data bits, and stop bits on a serial device.

  @param BaudRate           The requested baud rate. A BaudRate value of 0 will use the
                            device's default interface speed.
                            On output, the value actually set.
  @param ReveiveFifoDepth   The requested depth of the FIFO on the receive side of the
                            serial interface. A ReceiveFifoDepth value of 0 will use
                            the device's default FIFO depth.
                            On output, the value actually set.
  @param Timeout            The requested time out for a single character in microseconds.
                            This timeout applies to both the transmit and receive side of the
                            interface. A Timeout value of 0 will use the device's default time
                            out value.
                            On output, the value actually set.
  @param Parity             The type of parity to use on this serial device. A Parity value of
                            DefaultParity will use the device's default parity value.
                            On output, the value actually set.
  @param DataBits           The number of data bits to use on the serial device. A DataBits
                            vaule of 0 will use the device's default data bit setting.
                            On output, the value actually set.
  @param StopBits           The number of stop bits to use on this serial device. A StopBits
                            value of DefaultStopBits will use the device's default number of
                            stop bits.
                            On output, the value actually set.

  @retval RETURN_SUCCESS            The new attributes were set on the serial device.
  @retval RETURN_UNSUPPORTED        The serial device does not support this operation.
  @retval RETURN_INVALID_PARAMETER  One or more of the attributes has an unsupported value.
  @retval RETURN_DEVICE_ERROR       The serial device is not functioning correctly.

**/
RETURN_STATUS
EFIAPI
SerialPortSetAttributes (
  IN OUT UINT64              *BaudRate,
  IN OUT UINT32              *ReceiveFifoDepth,
  IN OUT UINT32              *Timeout,
  IN OUT EFI_PARITY_TYPE     *Parity,
  IN OUT UINT8               *DataBits,
  IN OUT EFI_STOP_BITS_TYPE  *StopBits
  )
{
  UINT32 Rate = *BaudRate;

  if (Rate != 0)
  {
    return SDM845v2SetBaud(FixedPcdGet32(PcdUartSerialBase), Rate);
  }

  return RETURN_UNSUPPORTED;
}
