#ifndef __LIBRARY_KEYPAD_DEVICE_IMPL_H__
#define __LIBRARY_KEYPAD_DEVICE_IMPL_H__

#include <Protocol/KeypadDevice.h>

EFI_STATUS EFIAPI KeypadDeviceImplReset (KEYPAD_DEVICE_PROTOCOL *This);
EFI_STATUS KeypadDeviceImplGetKeys (KEYPAD_DEVICE_PROTOCOL *This, KEYPAD_RETURN_API *KeypadReturnApi, UINT64 Delta);

#endif
