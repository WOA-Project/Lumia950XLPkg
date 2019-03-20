/*
 * Copyright (c) 2008, Google Inc.
 * All rights reserved.
 *
 * Copyright (c) 2009-2013, The Linux Foundation. All rights reserved.
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

#ifndef _HSUSB_H_
#define _HSUSB_H_

#define MSM_USB_BASE PcdGet64(PcdUsbHsBase)

#define USB_ID (MSM_USB_BASE + 0x0000)
#define USB_HWGENERAL (MSM_USB_BASE + 0x0004)
#define USB_HWHOST (MSM_USB_BASE + 0x0008)
#define USB_HWDEVICE (MSM_USB_BASE + 0x000C)
#define USB_HWTXBUF (MSM_USB_BASE + 0x0010)
#define USB_HWRXBUF (MSM_USB_BASE + 0x0014)
#define USB_SBUSCFG (MSM_USB_BASE + 0x0090)
#define USB_AHB_MODE (MSM_USB_BASE + 0x0098)

#define USB_GENCONFIG_2 (MSM_USB_BASE + 0x00A0)

#define USB_CAPLENGTH (MSM_USB_BASE + 0x0100)  /* 8 bit */
#define USB_HCIVERSION (MSM_USB_BASE + 0x0102) /* 16 bit */
#define USB_HCSPARAMS (MSM_USB_BASE + 0x0104)
#define USB_HCCPARAMS (MSM_USB_BASE + 0x0108)
#define USB_DCIVERSION (MSM_USB_BASE + 0x0120) /* 16 bit */
#define USB_USBCMD (MSM_USB_BASE + 0x0140)
#define USB_USBSTS (MSM_USB_BASE + 0x0144)
#define USB_USBINTR (MSM_USB_BASE + 0x0148)
#define USB_FRINDEX (MSM_USB_BASE + 0x014C)
#define USB_DEVICEADDR (MSM_USB_BASE + 0x0154)
#define USB_ENDPOINTLISTADDR (MSM_USB_BASE + 0x0158)
#define USB_BURSTSIZE (MSM_USB_BASE + 0x0160)
#define USB_TXFILLTUNING (MSM_USB_BASE + 0x0164)
#define USB_ULPI_VIEWPORT (MSM_USB_BASE + 0x0170)
#define USB_ENDPTNAK (MSM_USB_BASE + 0x0178)
#define USB_ENDPTNAKEN (MSM_USB_BASE + 0x017C)
#define USB_PORTSC (MSM_USB_BASE + 0x0184)
#define USB_OTGSC (MSM_USB_BASE + 0x01A4)
#define USB_USBMODE (MSM_USB_BASE + 0x01A8)
#define USB_ENDPTSETUPSTAT (MSM_USB_BASE + 0x01AC)
#define USB_ENDPTPRIME (MSM_USB_BASE + 0x01B0)
#define USB_ENDPTFLUSH (MSM_USB_BASE + 0x01B4)
#define USB_ENDPTSTAT (MSM_USB_BASE + 0x01B8)
#define USB_ENDPTCOMPLETE (MSM_USB_BASE + 0x01BC)
#define USB_ENDPTCTRL(n) (MSM_USB_BASE + 0x01C0 + (4 * (n)))
#define USB_OTG_HS_PHY_CTRL (MSM_USB_BASE + 0x0240)
#define USB_OTG_HS_PHY_SEC_CTRL (MSM_USB_BASE + 0x0278)

/* ULPI registers */
#define ULPI_MISC_A_READ 0x96
#define ULPI_MISC_A_SET 0x97
#define ULPI_MISC_A_CLEAR 0x98

#define USBCMD_RESET 2
#define USBCMD_ATTACH 1

#define USBMODE_DEVICE 2
#define USBMODE_HOST 3

struct ept_queue_head {
  unsigned config;
  unsigned current; /* read-only */

  unsigned next;
  unsigned info;
  unsigned page0;
  unsigned page1;
  unsigned page2;
  unsigned page3;
  unsigned page4;
  unsigned reserved_0;

  unsigned char setup_data[8];

  unsigned reserved_1;
  unsigned reserved_2;
  unsigned reserved_3;
  unsigned reserved_4;
};

#define CONFIG_MAX_PKT(n) ((n) << 16)
#define CONFIG_ZLT (1 << 29) /* stop on zero-len xfer */
#define CONFIG_IOS (1 << 15) /* IRQ on setup */

struct ept_queue_item {
  unsigned next;
  unsigned info;
  unsigned page0;
  unsigned page1;
  unsigned page2;
  unsigned page3;
  unsigned page4;
  unsigned reserved;
};

#define TERMINATE 1

#define INFO_BYTES(n) ((n) << 16)
#define INFO_IOC (1 << 15)
#define INFO_ACTIVE (1 << 7)
#define INFO_HALTED (1 << 6)
#define INFO_BUFFER_ERROR (1 << 5)
#define INFO_TX_ERROR (1 << 3)

#define STS_NAKI (1 << 16) /* */
#define STS_SLI (1 << 8)   /* R/WC - suspend state entered */
#define STS_SRI (1 << 7)   /* R/WC - SOF recv'd */
#define STS_URI (1 << 6)   /* R/WC - RESET recv'd - write to clear */
#define STS_FRI (1 << 3)   /* R/WC - Frame List Rollover */
#define STS_PCI (1 << 2)   /* R/WC - Port Change Detect */
#define STS_UEI (1 << 1)   /* R/WC - USB Error */
#define STS_UI (1 << 0)    /* R/WC - USB Transaction Complete */

/* bits used in all the endpoint status registers */
#define EPT_TX(n) (1 << ((n) + 16))
#define EPT_RX(n) (1 << (n))

#define CTRL_TXE (1 << 23)
#define CTRL_TXR (1 << 22)
#define CTRL_TXI (1 << 21)
#define CTRL_TXD (1 << 17)
#define CTRL_TXS (1 << 16)
#define CTRL_RXE (1 << 7)
#define CTRL_RXR (1 << 6)
#define CTRL_RXI (1 << 5)
#define CTRL_RXD (1 << 1)
#define CTRL_RXS (1 << 0)

#define CTRL_TXT_CTRL (0 << 18)
#define CTRL_TXT_ISOCH (1 << 18)
#define CTRL_TXT_BULK (2 << 18)
#define CTRL_TXT_INT (3 << 18)

#define CTRL_RXT_CTRL (0 << 2)
#define CTRL_RXT_ISOCH (1 << 2)
#define CTRL_RXT_BULK (2 << 2)
#define CTRL_RXT_INT (3 << 2)

#define GEN2_SESS_VLD_CTRL_EN (1 << 7)
#define SESS_VLD_CTRL (1 << 25)

/* ULPI bit map */
#define ULPI_WAKEUP (1 << 31)
#define ULPI_RUN (1 << 30)
#define ULPI_WRITE (1 << 29)
#define ULPI_READ (0 << 29)
#define ULPI_STATE_NORMAL (1 << 27)
#define ULPI_ADDR(n) (((n)&255) << 16)
#define ULPI_DATA(n) ((n)&255)
#define ULPI_DATA_READ(n) (((n) >> 8) & 255)

#define ULPI_MISC_A_VBUSVLDEXTSEL (1 << 1)
#define ULPI_MISC_A_VBUSVLDEXT (1 << 0)

#endif
