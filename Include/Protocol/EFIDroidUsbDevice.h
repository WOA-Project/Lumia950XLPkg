/** @file

  Copyright (c) 2013-2014, ARM Ltd. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
License which accompanies this distribution.  The full text of the license may
be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __EFIDROID_USB_DEVICE_PROTOCOL_H__
#define __EFIDROID_USB_DEVICE_PROTOCOL_H__

#include <IndustryStandard/Usb.h>

extern EFI_GUID gEFIDroidUsbDeviceProtocolGuid;

/*
 * Note: This Protocol is just  the bare minimum for Android Fastboot. It
 * only makes sense for devices that only do Bulk Transfers and only have one
 * endpoint.
 */

typedef enum {
  UsbDeviceEventDeviceStateChange,
  UsbDeviceEventTransferNotification
} USB_DEVICE_EVENT;

typedef enum {
  UsbDeviceTransferStatusCompleted,
  UsbDeviceTransferStatusFailed,
  UsbDeviceTransferStatusAborted,
} USB_DEVICE_TRANSFER_STATUS;

typedef enum {
  UsbDeviceStateConnected,
  UsbDeviceStateDisconnected
} USB_DEVICE_STATE;

typedef struct {
  USB_DEVICE_TRANSFER_STATUS Status;
  UINT8                      EndpointIndex;
  UINTN                      Size;
  VOID *                     Buffer;
} USB_DEVICE_TRANSFER_OUTCOME;

typedef union {
  USB_DEVICE_STATE            DeviceState;
  USB_DEVICE_TRANSFER_OUTCOME TransferOutcome;
} USB_DEVICE_EVENT_DATA;

typedef VOID (*USB_DEVICE_EVENT_CALLBACK)(
    IN USB_DEVICE_EVENT Event, IN USB_DEVICE_EVENT_DATA *EventData);

/*
  Put data in the Tx buffer to be sent on the next IN token.
  Don't call this function again until the TxCallback has been called.

  @param[in]Endpoint    Endpoint index, as specified in endpoint descriptors, of
                        the endpoint to send the data from.
  @param[in]Size        Size in bytes of data.
  @param[in]Buffer      Pointer to data.

  @retval EFI_SUCCESS           The data was queued successfully.
  @retval EFI_INVALID_PARAMETER There was an error sending the data.
*/
typedef EFI_STATUS (*USB_DEVICE_TRANSFER)(
    IN UINT8 EndpointIndex, IN UINTN Size, IN OUT VOID *Buffer);

/*
  Restart the USB peripheral controller and respond to enumeration.

  @param[in] DeviceDescriptor   pointer to device descriptor
  @param[in] Descriptors        Array of pointers to buffers, where
                                Descriptors[n] contains the response to a
                                GET_DESCRIPTOR request for configuration n. From
                                USB Spec section 9.4.3:
                                "The first interface descriptor follows the
                                configuration descriptor. The endpoint
                                descriptors for the first interface follow the
                                first interface descriptor. If there are
                                additional interfaces, their interface
                                descriptor and endpoint descriptors follow the
                                first interface’s endpoint descriptors".

                                The size of each buffer is the TotalLength
                                member of the Configuration Descriptor.

                                The size of the array is
                                DeviceDescriptor->NumConfigurations.
  @param[in]RxCallback          See USB_DEVICE_RX_CALLBACK
  @param[in]TxCallback          See USB_DEVICE_TX_CALLBACK
*/
typedef EFI_STATUS (*USB_DEVICE_START)(
    IN USB_DEVICE_DESCRIPTOR *DeviceDescriptor, IN VOID **Descriptors,
    IN USB_DEVICE_EVENT_CALLBACK EventCallback);

/*
  Stop the USB controller and clean up the resources allocated.
  After calling the function, no more access to the USB Device Protocol is
  allowed.

  @retval     EFI_SUCCESS           Function completed successfully.
  @retval     Others                Operation failed.
*/
typedef EFI_STATUS (*USB_DEVICE_STOP)(VOID);

/*
  Allocates a memory region of Size bytes and returns the address of the
  allocated memory that satisfies the underlying controller requirements in
  the location referenced by Buffer.
  The allocated transfer buffer must be freed using a matching call to
  the USB_DEVICE_FREE_TRANSFER_BUFFER function.

  @param[in]  Size              Number of bytes to allocate for the transfer
                                buffer.
  @param[out] Buffer            Pointer to a pointer to the allocated buffer
                                if the call succeeds; otherwise undefined.

  @retval     EFI_SUCCESS           Function completed successfully.
  @retval     EFI_INVALID_PARAMETER Parameter is invalid.
  @retval     EFI_OUT_OF_RESOURCES  Requested transfer buffer could not be
  allocated.
*/
typedef EFI_STATUS(EFIAPI *USB_DEVICE_ALLOCATE_TRANSFER_BUFFER)(
    IN UINTN Size, OUT VOID **Buffer);

/*
  Deallocates the memory specified by Buffer. The buffer that is freed
  must have been allocated by USB_DEVICE_ALLOCATE_TRANSFER_BUFFER.

  @param[in]  Buffer            Pointer to the transfer buffer to deallocate.

  @retval EFI_SUCCESS           Function completed successfully.
  @retval EFI_INVALID_PARAMETER Parameter is invalid.
*/
typedef EFI_STATUS(EFIAPI *USB_DEVICE_FREE_TRANSFER_BUFFER)(IN VOID *Buffer);

typedef EFI_STATUS (*USB_DEVICE_RESET_CONTROLLER)(VOID);

struct _USB_DEVICE_PROTOCOL {
  USB_DEVICE_START                    Start;
  USB_DEVICE_STOP                     Stop;
  USB_DEVICE_TRANSFER                 Transfer;
  USB_DEVICE_ALLOCATE_TRANSFER_BUFFER AllocateTransferBuffer;
  USB_DEVICE_FREE_TRANSFER_BUFFER     FreeTransferBuffer;
  USB_DEVICE_RESET_CONTROLLER         Reset;
};

typedef struct _USB_DEVICE_PROTOCOL USB_DEVICE_PROTOCOL;

#endif // ifndef __EFIDROID_USB_DEVICE_PROTOCOL_H__
