#include <PiDxe.h>

#include <Library/KeypadDeviceHelperLib.h>

UINTN gBitmapScanCodes[BITMAP_NUM_WORDS(MAX_SCANCODE)]       = {0};
UINTN gBitmapUnicodeChars[BITMAP_NUM_WORDS(MAX_UNCODE_CHAR)] = {0};

EFI_KEY_DATA gKeyDataPowerDown = {.Key = {
                                      .UnicodeChar = (CHAR16)'s',
                                  }};

EFI_KEY_DATA gKeyDataPowerUp = {.Key = {
                                    .UnicodeChar = (CHAR16)'e',
                                }};

EFI_KEY_DATA gKeyDataPowerLongpress = {.Key = {
                                           .UnicodeChar = (CHAR16)' ',
                                       }};
