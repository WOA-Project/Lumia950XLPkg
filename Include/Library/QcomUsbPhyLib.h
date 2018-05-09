#ifndef __LIBRARY_QCOM_USB_PHY_LIB_H__
#define __LIBRARY_QCOM_USB_PHY_LIB_H__

#include <Library/QcomPlatformUsbLib.h>

void usb30_qmp_phy_init(target_usb_iface_t *iface);
void qmp_phy_qmp_reset(target_usb_iface_t *iface);
bool use_hsonly_mode(void);

void qusb2_phy_reset(void);

#endif
