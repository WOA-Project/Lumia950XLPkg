/*
 * Copyright (c) 2009, Google Inc.
 * All rights reserved.
 * Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
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

#ifndef __DEV_UDC_H
#define __DEV_UDC_H

#include <IndustryStandard/Usb.h>
#include <Library/QcomPlatformUsbLib.h>

/* USB Device Controller Transfer Request */
struct udc_request {
  void *   buf;
  unsigned length;
  void (*complete)(struct udc_request *req, unsigned actual, int status);
  void *context;
};

/* endpoints are opaque handles specific to the particular device controller */
struct udc_endpoint;

struct udc_request *udc_request_alloc(void);
void                udc_request_free(struct udc_request *req);
int udc_request_queue(struct udc_endpoint *ept, struct udc_request *req);
int udc_request_cancel(struct udc_endpoint *ept, struct udc_request *req);

#define UDC_TYPE_BULK_IN 1
#define UDC_TYPE_BULK_OUT 2
#define UDC_TYPE_INTR_IN 3
#define UDC_TYPE_INTR_OUT 4

struct udc_endpoint *udc_endpoint_alloc(unsigned type, unsigned maxpkt);
struct udc_endpoint *
     udc_endpoint_alloc_raw(uint8_t in, unsigned type, unsigned maxpkt);
void udc_endpoint_free(struct udc_endpoint *ept);

#define UDC_EVENT_ONLINE 1
#define UDC_EVENT_OFFLINE 2

struct udc_gadget {
  void (*notify)(struct udc_gadget *gadget, unsigned event);
  void *context;

  USB_INTERFACE_DESCRIPTOR *InterfaceDescriptor;
  USB_CONFIG_DESCRIPTOR *   ConfigDescriptor;
  USB_ENDPOINT_DESCRIPTOR * EndpointDescriptors;

  struct udc_endpoint **ept;
};

struct udc_device {
  USB_DEVICE_DESCRIPTOR *DeviceDescriptor;
  target_usb_iface_t *   t_usb_if;
};

int udc_init(struct udc_device *devinfo);
int udc_register_gadget(struct udc_gadget *gadget);
int udc_start(void);
int udc_stop(void);

/* these should probably go elsewhere */
#define GET_STATUS 0
#define CLEAR_FEATURE 1
#define SET_FEATURE 3
#define SET_ADDRESS 5
#define GET_DESCRIPTOR 6
#define SET_DESCRIPTOR 7
#define GET_CONFIGURATION 8
#define SET_CONFIGURATION 9
#define GET_INTERFACE 10
#define SET_INTERFACE 11
#define SYNCH_FRAME 12
#define SET_SEL 48

#define TYPE_DEVICE 1
#define TYPE_CONFIGURATION 2
#define TYPE_STRING 3
#define TYPE_INTERFACE 4
#define TYPE_ENDPOINT 5
#define TYPE_BOS 15
#define TYPE_DEVICE_CAP 16
#define TYPE_SS_EP_COMP 48

#define DEVICE_READ 0x80
#define DEVICE_WRITE 0x00
#define INTERFACE_READ 0x81
#define INTERFACE_WRITE 0x01
#define ENDPOINT_READ 0x82
#define ENDPOINT_WRITE 0x02

#define TEST_SE0_NAK 0x0300
#define TEST_PACKET 0x0400
#define PORTSC_PTC (0xF << 16)
#define PORTSC_PTC_SE0_NAK (0x03 << 16)
#define PORTSC_PTC_TST_PKT (0x4 << 16)

#define USB_EP_NUM_MASK 0x0f
#define USB_EP_DIR_MASK 0x80
#define USB_EP_DIR_IN 0x80

struct setup_packet {
  unsigned char  type;
  unsigned char  request;
  unsigned short value;
  unsigned short index;
  unsigned short length;
} __attribute__((packed));

#endif
