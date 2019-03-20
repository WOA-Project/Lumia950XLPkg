#ifndef __LIBRARY_QCOM_PLATFORM_USB_LIB_H__
#define __LIBRARY_QCOM_PLATFORM_USB_LIB_H__

/* Target helper functions exposed to USB driver */
typedef struct target_usb_iface target_usb_iface_t;
struct target_usb_iface {
  CONST CHAR16 *controller;

  VOID (*usb_init)(target_usb_iface_t *iface);
  VOID (*usb_stop)(target_usb_iface_t *iface);
  VOID (*mux_config)(target_usb_iface_t *iface);
  VOID (*phy_reset)(target_usb_iface_t *iface);
  VOID (*phy_init)(target_usb_iface_t *iface);
  VOID (*clock_init)(target_usb_iface_t *iface);
  VOID (*clock_bumpup_pipe3_clk)(target_usb_iface_t *iface);

  UINT32 (*get_qmp_rev)(target_usb_iface_t *iface);

  BOOLEAN vbus_override;
  BOOLEAN pll_override;
};

EFI_STATUS LibQcomPlatformUsbGetInterface(target_usb_iface_t *iface);

#endif
