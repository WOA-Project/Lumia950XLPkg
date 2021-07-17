/*
 * Copyright (c) 2009, Google Inc.
 * All rights reserved.
 *
 * Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <Library/LKEnvLib.h>
#include <Library/MallocLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/StrLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <udc.h>

#include <IndustryStandard/Usb.h>
#include <Protocol/EFIDroidUsbDevice.h>

#include <hsusb.h>
#include <usb30_udc.h>

#define MAX_USBFS_BULK_SIZE (16 * 1024)
#define MAX_USBSS_BULK_SIZE (16 * 1024)

typedef struct {
  int (*udc_init)(struct udc_device *devinfo);
  int (*udc_register_gadget)(struct udc_gadget *gadget);
  int (*udc_start)(void);
  int (*udc_stop)(void);

  struct udc_endpoint *(*udc_endpoint_alloc)(unsigned type, unsigned maxpkt);
  struct udc_endpoint *(*udc_endpoint_alloc_raw)(
      uint8_t in, unsigned type, unsigned maxpkt);
  void (*udc_endpoint_free)(struct udc_endpoint *ept);
  struct udc_request *(*udc_request_alloc)(void);
  void (*udc_request_free)(struct udc_request *req);

  int (*udc_request_queue)(struct udc_endpoint *ept, struct udc_request *req);
  size_t max_usb_bulk_size;
} usb_controller_interface_t;

typedef enum {
  REQ_STATE_IDLE,
  REQ_STATE_TRANSFER,
} req_state_t;

typedef struct {
  LIST_ENTRY Link;

  UINT8                      EndpointIndex;
  USB_DEVICE_TRANSFER_STATUS TransferStatus;

  struct udc_request *req;
  VOID *              buf;
  UINTN               len;

  UINTN xfer_len;
  UINTN bytes_written;
} REQUEST_ENTRY;

typedef struct {
  struct udc_endpoint *    udc_ept;
  USB_ENDPOINT_DESCRIPTOR *EPDesc;
  BOOLEAN                  IsIn;
} IENDPOINT;

STATIC VOID gadget_notify(struct udc_gadget *gadget, unsigned event);
STATIC VOID req_complete(struct udc_request *req, unsigned actual, int status);

STATIC VOID *mDescriptors;
STATIC USB_DEVICE_DESCRIPTOR *mDeviceDescriptor;
STATIC USB_INTERFACE_DESCRIPTOR *mInterfaceDescriptor;
STATIC USB_CONFIG_DESCRIPTOR *mConfigDescriptor;
STATIC USB_ENDPOINT_DESCRIPTOR *mEndpointDescriptors;

STATIC EFI_EVENT          mEventsAvailable;
USB_DEVICE_EVENT_CALLBACK mEventCallback;

STATIC usb_controller_interface_t mUsbIf;
STATIC struct udc_request *       mUdcRequest;
STATIC req_state_t                mRequestState = REQ_STATE_IDLE;
STATIC LIST_ENTRY mRequestQueue = INITIALIZE_LIST_HEAD_VARIABLE(mRequestQueue);
STATIC LIST_ENTRY mFinishedRequestQueue =
    INITIALIZE_LIST_HEAD_VARIABLE(mFinishedRequestQueue);
STATIC IENDPOINT *      mEndpoints         = NULL;
STATIC BOOLEAN          mReportDeviceState = FALSE;
STATIC USB_DEVICE_STATE mDeviceState       = UsbDeviceStateDisconnected;
STATIC EFI_LOCK         mLock = EFI_INITIALIZE_LOCK_VARIABLE(TPL_HIGH_LEVEL);

STATIC struct udc_device mUdcDevice;
STATIC struct udc_gadget mGadget = {
    .notify              = gadget_notify,
    .InterfaceDescriptor = NULL,
    .ConfigDescriptor    = NULL,
    .EndpointDescriptors = NULL,
    .ept                 = NULL,
};

STATIC EFI_STATUS req_queue_request(REQUEST_ENTRY *RequestEntry)
{
  INTN       r;
  EFI_STATUS Status;

  RequestEntry->xfer_len = RequestEntry->len - RequestEntry->bytes_written;
  if (RequestEntry->xfer_len > mUsbIf.max_usb_bulk_size)
    RequestEntry->xfer_len = mUsbIf.max_usb_bulk_size;

  RequestEntry->req->buf =
      (VOID *)(UINTN)RequestEntry->buf + RequestEntry->bytes_written;
  RequestEntry->req->length   = RequestEntry->xfer_len;
  RequestEntry->req->complete = req_complete;

  r = mUsbIf.udc_request_queue(
      mGadget.ept[RequestEntry->EndpointIndex], RequestEntry->req);
  if (r < 0) {
    DEBUG((CRITICAL, "udc_request_queue() failed. r = %d\n", r));
    RequestEntry->TransferStatus = UsbDeviceTransferStatusFailed;

    mRequestState = REQ_STATE_IDLE;
    RemoveEntryList(&RequestEntry->Link);
    InsertTailList(&mFinishedRequestQueue, &RequestEntry->Link);

    // signal
    gBS->SignalEvent(mEventsAvailable);

    Status = EFI_DEVICE_ERROR;
  }
  else {
    mRequestState = REQ_STATE_TRANSFER;
    Status        = EFI_SUCCESS;
  }

  return Status;
}

STATIC VOID
req_handle_transfer(REQUEST_ENTRY *RequestEntry, UINTN actual, INTN status)
{
  if (status < 0) {
    DEBUG((DEBUG_ERROR, "transfer error: %d\n", status));
    RequestEntry->TransferStatus = UsbDeviceTransferStatusFailed;

    mRequestState = REQ_STATE_IDLE;
    RemoveEntryList(&RequestEntry->Link);
    InsertTailList(&mFinishedRequestQueue, &RequestEntry->Link);

    // signal
    gBS->SignalEvent(mEventsAvailable);

    return;
  }
  RequestEntry->bytes_written += actual;

  if (actual == RequestEntry->xfer_len) {
    // read successful

    if (RequestEntry->bytes_written != RequestEntry->len) {
      // queue next chunk
      req_queue_request(RequestEntry);
      return;
    }
  }
  else {
    // short transfer
    if (mEndpoints[RequestEntry->EndpointIndex].IsIn) {
      // short write
      DEBUG((DEBUG_ERROR, "short write\n"));
      RequestEntry->TransferStatus = UsbDeviceTransferStatusFailed;

      mRequestState = REQ_STATE_IDLE;
      RemoveEntryList(&RequestEntry->Link);
      InsertTailList(&mFinishedRequestQueue, &RequestEntry->Link);

      // signal
      gBS->SignalEvent(mEventsAvailable);

      return;
    }
  }

  // invalidate any cached buf data (controller updates main memory)
  if (!mEndpoints[RequestEntry->EndpointIndex].IsIn)
    arch_invalidate_cache_range((UINTN)RequestEntry->buf, RequestEntry->len);

  RequestEntry->TransferStatus = UsbDeviceTransferStatusCompleted;

  mRequestState = REQ_STATE_IDLE;
  RemoveEntryList(&RequestEntry->Link);
  InsertTailList(&mFinishedRequestQueue, &RequestEntry->Link);

  // signal
  gBS->SignalEvent(mEventsAvailable);
}

STATIC VOID req_complete(struct udc_request *req, unsigned actual, int status)
{
  if (IsListEmpty(&mRequestQueue)) {
    DEBUG(
        (DEBUG_ERROR, "unexpected req_complete(%p, %u, %d)\n", req, actual,
         status));
    return;
  }

  REQUEST_ENTRY *RequestEntry = (REQUEST_ENTRY *)GetFirstNode(&mRequestQueue);
  if (RequestEntry->req != req) {
    DEBUG(
        (DEBUG_ERROR, "invalid req_complete(%p, %u, %d), expected req=%p\n",
         req, actual, status, RequestEntry->req));
    return;
  }

  switch (mRequestState) {
  case REQ_STATE_TRANSFER:
    req_handle_transfer(RequestEntry, actual, status);
    break;

  default:
    DEBUG((DEBUG_ERROR, "Invalid request state: %d\n", mRequestState));
    break;
  }

  while (mRequestState == REQ_STATE_IDLE && !IsListEmpty(&mRequestQueue)) {
    REQUEST_ENTRY *RequestEntry = (REQUEST_ENTRY *)GetFirstNode(&mRequestQueue);
    req_queue_request(RequestEntry);
  }
}

STATIC VOID gadget_notify(struct udc_gadget *gadget, unsigned event)
{
  if (event == UDC_EVENT_ONLINE) {
    EfiAcquireLock(&mLock);
    if (mDeviceState != UsbDeviceStateConnected) {
      mReportDeviceState = TRUE;
      mDeviceState       = UsbDeviceStateConnected;

      // signal
      gBS->SignalEvent(mEventsAvailable);
    }
    EfiReleaseLock(&mLock);
  }

  else if (event == UDC_EVENT_OFFLINE) {
    EfiAcquireLock(&mLock);

    if (mDeviceState != UsbDeviceStateDisconnected) {
      mReportDeviceState = TRUE;
      mDeviceState       = UsbDeviceStateDisconnected;
    }

    // delete all requests from the queue
    REQUEST_ENTRY *FirstRequestEntry =
        (REQUEST_ENTRY *)GetFirstNode(&mRequestQueue);
    while (!IsListEmpty(&mRequestQueue)) {
      REQUEST_ENTRY *RequestEntry =
          (REQUEST_ENTRY *)GetFirstNode(&mRequestQueue);
      RemoveEntryList(&RequestEntry->Link);
      InsertTailList(&mFinishedRequestQueue, &RequestEntry->Link);
    }

    if (&FirstRequestEntry->Link != &mRequestQueue &&
        mRequestState == REQ_STATE_TRANSFER) {
      // add back head entry if it's currently in the udc queue.
      // we'll receive a request failed event later
      RemoveEntryList(&FirstRequestEntry->Link);
      InsertTailList(&mRequestQueue, &FirstRequestEntry->Link);
    }

    EfiReleaseLock(&mLock);

    // signal
    gBS->SignalEvent(mEventsAvailable);
  }
}

STATIC EFI_STATUS usb_init(VOID)
{
  EFI_STATUS               Status;
  USB_ENDPOINT_DESCRIPTOR *EPDesc;
  INTN                     rc;

  memset(&mUdcDevice, 0, sizeof(mUdcDevice));
  mUdcDevice.DeviceDescriptor = mDeviceDescriptor;

  mUdcDevice.t_usb_if = AllocateZeroPool(sizeof(target_usb_iface_t));
  if (!mUdcDevice.t_usb_if) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = LibQcomPlatformUsbGetInterface(mUdcDevice.t_usb_if);
  if (EFI_ERROR(Status)) {
    goto fail_get_interface;
  }

  if (!StrCmp(mUdcDevice.t_usb_if->controller, L"dwc")) {
    /* initialize udc functions to use dwc controller */
    mUsbIf.udc_init            = usb30_udc_init;
    mUsbIf.udc_register_gadget = usb30_udc_register_gadget;
    mUsbIf.udc_start           = usb30_udc_start;
    mUsbIf.udc_stop            = usb30_udc_stop;

    mUsbIf.udc_endpoint_alloc     = usb30_udc_endpoint_alloc;
    mUsbIf.udc_endpoint_alloc_raw = usb30_udc_endpoint_alloc_raw;
    mUsbIf.udc_request_alloc      = usb30_udc_request_alloc;
    mUsbIf.udc_request_free       = usb30_udc_request_free;

    mUsbIf.udc_request_queue = usb30_udc_request_queue;
    mUsbIf.max_usb_bulk_size = MAX_USBSS_BULK_SIZE;
  }
#ifdef MDE_CPU_ARM
  else if (!StrCmp(mUdcDevice.t_usb_if->controller, L"ci")) {
    /* initialize udc functions to use the default chipidea controller */
    mUsbIf.udc_init            = udc_init;
    mUsbIf.udc_register_gadget = udc_register_gadget;
    mUsbIf.udc_start           = udc_start;
    mUsbIf.udc_stop            = udc_stop;

    mUsbIf.udc_endpoint_alloc     = udc_endpoint_alloc;
    mUsbIf.udc_endpoint_alloc_raw = udc_endpoint_alloc_raw;
    mUsbIf.udc_request_alloc      = udc_request_alloc;
    mUsbIf.udc_request_free       = udc_request_free;

    mUsbIf.udc_request_queue = udc_request_queue;
    mUsbIf.max_usb_bulk_size = MAX_USBFS_BULK_SIZE;
  }
#endif
  else {
    Status = EFI_UNSUPPORTED;
    goto fail_unsupported_controller;
  }

  /* register udc device */
  rc = mUsbIf.udc_init(&mUdcDevice);
  if (rc) {
    Status = EFI_DEVICE_ERROR;
    goto fail_udc_init;
  }

  mGadget.InterfaceDescriptor = mInterfaceDescriptor;
  mGadget.ConfigDescriptor    = mConfigDescriptor;
  mGadget.EndpointDescriptors = mEndpointDescriptors;

  mGadget.ept = AllocateZeroPool(
      sizeof(struct udc_endpoint *) * mInterfaceDescriptor->NumEndpoints);
  if (!mGadget.ept) {
    Status = EFI_OUT_OF_RESOURCES;
    goto fail_ept_alloc;
  }

  mEndpoints =
      AllocateZeroPool(sizeof(IENDPOINT) * mInterfaceDescriptor->NumEndpoints);
  if (!mEndpoints) {
    Status = EFI_OUT_OF_RESOURCES;
    goto fail_iept_alloc;
  }

  for (UINT8 n = 0; n < mInterfaceDescriptor->NumEndpoints; n++) {
    EPDesc = &mEndpointDescriptors[n];

    UINT8 _in      = EPDesc->EndpointAddress & BIT7;
    mGadget.ept[n] = mUsbIf.udc_endpoint_alloc_raw(
        !!_in, EPDesc->Attributes, EPDesc->MaxPacketSize);
    if (!mGadget.ept[n]) {
      Status = EFI_OUT_OF_RESOURCES;
      goto fail_udc_ept_alloc;
    }

    mEndpoints[n].udc_ept = mGadget.ept[n];
    mEndpoints[n].EPDesc  = EPDesc;
    mEndpoints[n].IsIn    = _in;
  }

  mUdcRequest = mUsbIf.udc_request_alloc();
  if (!mUdcRequest)
    goto fail_alloc_req;

  /* register gadget */
  if (mUsbIf.udc_register_gadget(&mGadget))
    goto fail_udc_register;

  mUsbIf.udc_start();

  return EFI_SUCCESS;

fail_udc_register:
  mUsbIf.udc_request_free(mUdcRequest);
fail_alloc_req:
fail_udc_ept_alloc:
  for (UINT8 n = 0; n < mInterfaceDescriptor->NumEndpoints; n++) {
    if (mGadget.ept[n])
      mUsbIf.udc_endpoint_free(mGadget.ept[n]);
  }
fail_iept_alloc:
fail_ept_alloc:
fail_udc_init:
fail_unsupported_controller:
fail_get_interface:
  FreePool(mUdcDevice.t_usb_if);
  return Status;
}

EFI_STATUS
EFIAPI
Usb30PeriphStart(
    IN USB_DEVICE_DESCRIPTOR *DeviceDescriptor, IN VOID **Descriptors,
    IN USB_DEVICE_EVENT_CALLBACK EventCallback)
{
  UINT8 *Ptr;

  ASSERT(DeviceDescriptor != NULL);
  ASSERT(Descriptors[0] != NULL);
  ASSERT(EventCallback != NULL);

  mDeviceDescriptor = DeviceDescriptor;
  mDescriptors      = Descriptors[0];

  // Right now we just support one configuration
  ASSERT(mDeviceDescriptor->NumConfigurations == 1);
  // ... and one interface
  mConfigDescriptor = (USB_CONFIG_DESCRIPTOR *)mDescriptors;
  ASSERT(mConfigDescriptor->NumInterfaces == 1);

  Ptr = ((UINT8 *)mDescriptors) + sizeof(USB_CONFIG_DESCRIPTOR);
  mInterfaceDescriptor = (USB_INTERFACE_DESCRIPTOR *)Ptr;
  Ptr += sizeof(USB_INTERFACE_DESCRIPTOR);

  mEndpointDescriptors = (USB_ENDPOINT_DESCRIPTOR *)Ptr;

  mEventCallback = EventCallback;

  return usb_init();
}

EFI_STATUS
Usb30PeriphStop(VOID)
{
  mUsbIf.udc_stop();
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
Usb30PeriphTransfer(IN UINT8 EndpointIndex, IN UINTN Size, IN VOID *Buffer)
{
  REQUEST_ENTRY *RequestEntry = AllocatePool(sizeof(REQUEST_ENTRY));
  if (!RequestEntry)
    return EFI_OUT_OF_RESOURCES;

  RequestEntry->EndpointIndex = EndpointIndex;
  RequestEntry->req           = mUdcRequest;
  RequestEntry->buf           = Buffer;
  RequestEntry->len           = Size;
  RequestEntry->bytes_written = 0;

  if (mEndpoints[RequestEntry->EndpointIndex].IsIn)
    // flush buffer to main memory before giving to udc
    arch_clean_invalidate_cache_range(
        (UINTN)RequestEntry->buf, RequestEntry->len);
  else
    // discard the cache contents before starting the download
    arch_invalidate_cache_range((UINTN)RequestEntry->buf, RequestEntry->len);

  EfiAcquireLock(&mLock);
  InsertTailList(&mRequestQueue, &RequestEntry->Link);
  if (mRequestState == REQ_STATE_IDLE)
    req_queue_request(RequestEntry);
  EfiReleaseLock(&mLock);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
UsbDeviceAllocateTransferBuffer(IN UINTN Size, OUT VOID **Buffer)
{
  VOID *Memory = memalign(CACHE_LINE, ROUNDUP(Size, CACHE_LINE));
  if (!Memory) {
    return EFI_OUT_OF_RESOURCES;
  }

  *Buffer = Memory;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
UsbDeviceFreeTransferBuffer(IN VOID *Buffer)
{
  free(Buffer);
  return EFI_SUCCESS;
}

STATIC USB_DEVICE_PROTOCOL mUsbDevice = {
    Usb30PeriphStart, Usb30PeriphStop, Usb30PeriphTransfer,
    UsbDeviceAllocateTransferBuffer, UsbDeviceFreeTransferBuffer};

STATIC
VOID ReportEvents(IN EFI_EVENT Event, IN VOID *Context)
{
  USB_DEVICE_EVENT_DATA EventData;

  EfiAcquireLock(&mLock);

  // report device state
  if (mReportDeviceState) {
    mReportDeviceState = FALSE;

    EfiReleaseLock(&mLock);
    EventData.DeviceState = mDeviceState;
    mEventCallback(UsbDeviceEventDeviceStateChange, &EventData);
    EfiAcquireLock(&mLock);
  }

  // report finished requests
  while (!IsListEmpty(&mFinishedRequestQueue)) {
    REQUEST_ENTRY *RequestEntry =
        (REQUEST_ENTRY *)GetFirstNode(&mFinishedRequestQueue);
    RemoveEntryList(&RequestEntry->Link);

    EfiReleaseLock(&mLock);
    EventData.TransferOutcome.Status        = RequestEntry->TransferStatus;
    EventData.TransferOutcome.EndpointIndex = RequestEntry->EndpointIndex;
    EventData.TransferOutcome.Buffer        = RequestEntry->buf;
    EventData.TransferOutcome.Size          = RequestEntry->bytes_written;

    mEventCallback(UsbDeviceEventTransferNotification, &EventData);

    FreePool(RequestEntry);
    EfiAcquireLock(&mLock);
  }

  EfiReleaseLock(&mLock);
}

EFI_STATUS
EFIAPI
UsbEntryPoint(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;
  EFI_HANDLE Handle;

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL, TPL_CALLBACK, ReportEvents, NULL, &mEventsAvailable);
  ASSERT_EFI_ERROR(Status);

  Handle = NULL;
  return gBS->InstallProtocolInterface(
      &Handle, &gEFIDroidUsbDeviceProtocolGuid, EFI_NATIVE_INTERFACE,
      &mUsbDevice);
}
