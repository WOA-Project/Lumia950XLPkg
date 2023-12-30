#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomGpioTlmmLib.h>
#include <Library/QcomPm8x41Lib.h>

#include <Library/KeypadDeviceHelperLib.h>
#include <Library/KeypadDeviceImplLib.h>
#include <Protocol/KeypadDevice.h>

typedef enum {
  KEY_DEVICE_TYPE_UNKNOWN,

  KEY_DEVICE_TYPE_TLMM,
  KEY_DEVICE_TYPE_PM8X41,
  KEY_DEVICE_TYPE_PM8X41_PON,
} KEY_DEVICE_TYPE;

typedef struct {
  KEY_CONTEXT     EfiKeyContext;
  BOOLEAN         IsValid;
  KEY_DEVICE_TYPE DeviceType;

  // gpio
  UINT8   Gpio;
  BOOLEAN ActiveLow;

  // pon
  UINT32 PonType;
} KEY_CONTEXT_PRIVATE;

STATIC KEY_CONTEXT_PRIVATE KeyContextPower;
STATIC KEY_CONTEXT_PRIVATE KeyContextVolumeUp;
STATIC KEY_CONTEXT_PRIVATE KeyContextVolumeDown;
STATIC KEY_CONTEXT_PRIVATE KeyContextCamera;

STATIC KEY_CONTEXT_PRIVATE *KeyList[] = {&KeyContextPower, &KeyContextVolumeUp,
                                         &KeyContextVolumeDown,
                                         &KeyContextCamera};

STATIC
VOID KeypadInitializeKeyContextPrivate(KEY_CONTEXT_PRIVATE *Context)
{
  Context->IsValid    = FALSE;
  Context->Gpio       = 0;
  Context->DeviceType = KEY_DEVICE_TYPE_UNKNOWN;
  Context->ActiveLow  = FALSE;
}

STATIC
KEY_CONTEXT_PRIVATE *KeypadKeyCodeToKeyContext(UINT32 KeyCode)
{
  if (KeyCode == 114)
    return &KeyContextVolumeDown;
  else if (KeyCode == 115)
    return &KeyContextVolumeUp;
  else if (KeyCode == 116)
    return &KeyContextPower;
  else if (KeyCode == 766)
    return &KeyContextCamera;
  else
    return NULL;
}

RETURN_STATUS
EFIAPI
KeypadDeviceImplConstructor(VOID)
{
  UINTN                Index;
  KEY_CONTEXT_PRIVATE *StaticContext;

  // Reset all keys
  for (Index = 0; Index < ARRAY_SIZE(KeyList); Index++) {
    KeypadInitializeKeyContextPrivate(KeyList[Index]);
  }

  // Configure keys

  // Vol Up (115) , Camera Splash (766) and Camera Focus (528)
  // go through PMIC GPIO
  StaticContext             = KeypadKeyCodeToKeyContext(115);
  StaticContext->DeviceType = KEY_DEVICE_TYPE_PM8X41;
  StaticContext->Gpio       = 3;
  StaticContext->ActiveLow  = 0x1 & 0x1;
  StaticContext->IsValid    = TRUE;

  StaticContext             = KeypadKeyCodeToKeyContext(766);
  StaticContext->DeviceType = KEY_DEVICE_TYPE_PM8X41;
  StaticContext->Gpio       = 4;
  StaticContext->ActiveLow  = 0x1 & 0x1;
  StaticContext->IsValid    = TRUE;

  // Vol Down (114) and Power On (116) on through PMIC PON
  StaticContext             = KeypadKeyCodeToKeyContext(114);
  StaticContext->DeviceType = KEY_DEVICE_TYPE_PM8X41_PON;
  StaticContext->PonType    = 1;
  StaticContext->IsValid    = TRUE;

  StaticContext             = KeypadKeyCodeToKeyContext(116);
  StaticContext->DeviceType = KEY_DEVICE_TYPE_PM8X41_PON;
  StaticContext->PonType    = 0;
  StaticContext->IsValid    = TRUE;

  return RETURN_SUCCESS;
}

EFI_STATUS EFIAPI KeypadDeviceImplReset(KEYPAD_DEVICE_PROTOCOL *This)
{
  LibKeyInitializeKeyContext(&KeyContextPower.EfiKeyContext);
  KeyContextPower.EfiKeyContext.KeyData.Key.UnicodeChar = CHAR_CARRIAGE_RETURN;

  LibKeyInitializeKeyContext(&KeyContextVolumeUp.EfiKeyContext);
  KeyContextVolumeUp.EfiKeyContext.KeyData.Key.ScanCode = SCAN_UP;

  LibKeyInitializeKeyContext(&KeyContextVolumeDown.EfiKeyContext);
  KeyContextVolumeDown.EfiKeyContext.KeyData.Key.ScanCode = SCAN_DOWN;

  LibKeyInitializeKeyContext(&KeyContextCamera.EfiKeyContext);
  KeyContextCamera.EfiKeyContext.KeyData.Key.ScanCode = SCAN_ESC;

  return EFI_SUCCESS;
}

EFI_STATUS KeypadDeviceImplGetKeys(
    KEYPAD_DEVICE_PROTOCOL *This, KEYPAD_RETURN_API *KeypadReturnApi,
    UINT64 Delta)
{
  UINT8   GpioStatus;
  BOOLEAN IsPressed;
  INTN    RC;
  UINTN   Index;

  for (Index = 0; Index < ARRAY_SIZE(KeyList); Index++) {
    KEY_CONTEXT_PRIVATE *Context = KeyList[Index];

    // check if this is a valid key
    if (Context->IsValid == FALSE)
      continue;

    // get status
    if (Context->DeviceType == KEY_DEVICE_TYPE_TLMM) {
      GpioStatus = gGpioTlmm->Get(Context->Gpio);
      RC         = 0;
    }
    else if (Context->DeviceType == KEY_DEVICE_TYPE_PM8X41) {
      RC = gPm8x41->pm8x41_gpio_get(Context->Gpio, &GpioStatus);
    }
    else if (Context->DeviceType == KEY_DEVICE_TYPE_PM8X41_PON) {
      if (Context->PonType == 0x1)
        GpioStatus = gPm8x41->pm8x41_resin_status();
      else if (Context->PonType == 0x0)
        GpioStatus = gPm8x41->pm8x41_get_pwrkey_is_pressed();
      else
        continue;

      RC = 0;
    }
    else {
      continue;
    }
    if (RC != 0)
      continue;

    // update key status
    IsPressed = (GpioStatus ? 1 : 0) ^ Context->ActiveLow;
    LibKeyUpdateKeyStatus(
        &Context->EfiKeyContext, KeypadReturnApi, IsPressed, Delta);
  }

  return EFI_SUCCESS;
}
