#ifndef __LIBRARY_KEYPAD_DEVICE_HELPER_H__
#define __LIBRARY_KEYPAD_DEVICE_HELPER_H__

#include <Library/DebugLib.h>
#include <Library/BitmapLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/KeypadDevice.h>

#define MAX_SCANCODE 0x18
#define MAX_UNCODE_CHAR 0x7f
#define MS2NS(ms) (((UINT64)(ms)) * 1000000ULL)

typedef enum {
  KEYSTATE_RELEASED,
  KEYSTATE_PRESSED,
  KEYSTATE_LONGPRESS_RELEASE,
} KEY_STATE;

typedef struct {
  // keydata to be send to the driver
  EFI_KEY_DATA KeyData;

  // the time the current action has been running
  UINT64 Time;

  // the current state of the key
  KEY_STATE State;

  // the current key has to repeat sending events
  BOOLEAN Repeat;

  // the user did a longpress
  BOOLEAN Longpress;
} KEY_CONTEXT;

extern UINTN gBitmapScanCodes[BITMAP_NUM_WORDS(MAX_SCANCODE)];
extern UINTN gBitmapUnicodeChars[BITMAP_NUM_WORDS(MAX_UNCODE_CHAR)];

extern EFI_KEY_DATA gKeyDataPowerDown;
extern EFI_KEY_DATA gKeyDataPowerUp;
extern EFI_KEY_DATA gKeyDataPowerLongpress;

STATIC inline VOID KeySetState(UINT16 ScanCode, CHAR16 UnicodeChar, BOOLEAN Value)
{
  if (ScanCode && ScanCode<MAX_SCANCODE) {
    if (Value)
      BitmapSet(gBitmapScanCodes, ScanCode);
    else
      BitmapClear(gBitmapScanCodes, ScanCode);
  }

  if (UnicodeChar && UnicodeChar<MAX_UNCODE_CHAR) {
    if (Value)
      BitmapSet(gBitmapUnicodeChars, ScanCode);
    else
      BitmapClear(gBitmapUnicodeChars, ScanCode);
  }
}

STATIC inline BOOLEAN KeyGetState(UINT16 ScanCode, CHAR16 UnicodeChar)
{
  if (ScanCode && ScanCode<MAX_SCANCODE) {
    if (!BitmapTest(gBitmapScanCodes, ScanCode))
      return FALSE;
  }

  if (UnicodeChar && UnicodeChar<MAX_UNCODE_CHAR) {
    if (!BitmapTest(gBitmapUnicodeChars, ScanCode))
      return FALSE;
  }

  return TRUE;
}

STATIC inline VOID LibKeyInitializeKeyContext(KEY_CONTEXT *Context) {
  SetMem(&Context->KeyData, sizeof(Context->KeyData), 0);
  Context->Time = 0;
  Context->State = KEYSTATE_RELEASED;
  Context->Repeat = FALSE;
  Context->Longpress = FALSE;
}

STATIC inline VOID LibKeyUpdateKeyStatus(KEY_CONTEXT *Context, KEYPAD_RETURN_API *KeypadReturnApi, BOOLEAN IsPressed, UINT64 Delta) {
  // keep track of the actual state
  KeySetState(Context->KeyData.Key.ScanCode, Context->KeyData.Key.UnicodeChar, IsPressed);

  // update key time
  Context->Time += Delta;

  switch (Context->State) {
    case KEYSTATE_RELEASED:
      if (IsPressed) {
          // change to pressed
          Context->Time = 0;
          Context->State = KEYSTATE_PRESSED;
      }
      break;

    case KEYSTATE_PRESSED:
      if (IsPressed) {
        // keyrepeat
        if (Context->Repeat && Context->Time>=MS2NS(200)) {
          KeypadReturnApi->PushEfikeyBufTail(KeypadReturnApi, &Context->KeyData);
          Context->Time = 0;
          Context->Repeat = TRUE;
        }

        else if (!Context->Longpress && Context->Time>=MS2NS(500)) {
          // POWER, handle key combos
          if (Context->KeyData.Key.UnicodeChar==CHAR_CARRIAGE_RETURN) {
            if (KeyGetState(SCAN_DOWN, 0)) {
              // report 's'
              KeypadReturnApi->PushEfikeyBufTail(KeypadReturnApi, &gKeyDataPowerDown);
            } else if (KeyGetState(SCAN_UP, 0)) {
              // report 'e'
              KeypadReturnApi->PushEfikeyBufTail(KeypadReturnApi, &gKeyDataPowerUp);
            } else {
              // report spacebar
              KeypadReturnApi->PushEfikeyBufTail(KeypadReturnApi, &gKeyDataPowerLongpress);
            }
          }

          // post first keyrepeat event
          else {
            // only start keyrepeat if we're not doing a combo
            if (!KeyGetState(0, CHAR_CARRIAGE_RETURN)) {
              KeypadReturnApi->PushEfikeyBufTail(KeypadReturnApi, &Context->KeyData);
              Context->Time = 0;
              Context->Repeat = TRUE;
            }
          }

          Context->Longpress = TRUE;
        }
      }

      else {
        if (!Context->Longpress) {
          // we supressed down, so report it now
          KeypadReturnApi->PushEfikeyBufTail(KeypadReturnApi, &Context->KeyData);
          Context->State = KEYSTATE_LONGPRESS_RELEASE;
        }

        else if (Context->Time>=MS2NS(10)) {
          // we reported another key already
          Context->Time = 0;
          Context->Repeat = FALSE;
          Context->Longpress = FALSE;
          Context->State = KEYSTATE_RELEASED;
        }
      }
      break;

    case KEYSTATE_LONGPRESS_RELEASE:
      // change to released
      Context->Time = 0;
      Context->Repeat = FALSE;
      Context->Longpress = FALSE;
      Context->State = KEYSTATE_RELEASED;
      break;

    default:
      ASSERT(FALSE);
      break;
  }
}

#endif
