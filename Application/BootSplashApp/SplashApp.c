#include <Uefi.h>

#include <IndustryStandard/Bmp.h>
#include <IndustryStandard/SmBios.h>

#include <Guid/MdeModuleHii.h>
#include <Guid/StatusCodeDataTypeId.h>

#include <Library/BaseMemoryLib.h>
#include <Library/BgraRgbaConvert.h>
#include <Library/BootAppLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/lodepng.h>

#include <Pi/PiFirmwareFile.h>

#include <Protocol/AbsolutePointer.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextInEx.h>

EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *mSimpleTextInputProtocol;
EFI_ABSOLUTE_POINTER_PROTOCOL *    mAbsPointerProtocol;
EFI_EVENT                          mPollingEvent;
EFI_EVENT                          mWaitingEvent;
EFI_GRAPHICS_OUTPUT_PROTOCOL *     mGop;

BOOLEAN mBootTorEFIndApp  = FALSE;
UINTN   mCounter          = 0;
UINTN   mCounterThreshold = 0;

STATIC EFI_GUID gSplashPromptGuid = {
    0x5af45c60,
    0xa0cd,
    0x4aab,
    {
        0xb6,
        0xf4,
        0xb1,
        0xc0,
        0x7c,
        0x34,
        0x7a,
        0xd7,
    },
};

STATIC EFI_GUID gSplashPromptClearGuid = {
    0x5af45c60,
    0xa0cd,
    0x4aab,
    {
        0xb6,
        0xf4,
        0xb1,
        0xc0,
        0x7c,
        0x34,
        0x7a,
        0xd8,
    },
};

VOID EFIAPI InputPollingCallback(IN EFI_EVENT Event, IN VOID *Context)
{
  EFI_STATUS                 Status;
  EFI_KEY_DATA               KeyData;
  EFI_ABSOLUTE_POINTER_STATE PointerState;
  Status = mSimpleTextInputProtocol->ReadKeyStrokeEx(
      mSimpleTextInputProtocol, &KeyData);
  if (!EFI_ERROR(Status)) {
    if (KeyData.Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      mBootTorEFIndApp = TRUE;
    }
  }
  Status = mAbsPointerProtocol->GetState(mAbsPointerProtocol, &PointerState);
  if (!EFI_ERROR(Status)) {
    // Tapping anywhere is okay
    mBootTorEFIndApp = TRUE;
  }

  if (mCounter >= mCounterThreshold || mBootTorEFIndApp) {
    gBS->CloseEvent(mPollingEvent);
    gBS->SignalEvent(mWaitingEvent);
  }

  mCounter++;
}

EFI_STATUS EFIAPI DiscoverAndBootApp(
    IN EFI_HANDLE ImageHandle, CHAR16 *AppPath, CHAR16 *FallbackPath)
{
  EFI_HANDLE *               FileSystemHandles;
  EFI_LOADED_IMAGE_PROTOCOL *LoadedAppImage;
  EFI_HANDLE                 LoadedAppHandle;
  UINTN                      NumberFileSystemHandles;
  EFI_STATUS                 Status;
  EFI_DEVICE_PATH_PROTOCOL * FilePath = NULL;

  Status = gBS->LocateHandleBuffer(
      ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL,
      &NumberFileSystemHandles, &FileSystemHandles);

  for (UINTN Handle = 0; Handle < NumberFileSystemHandles; Handle++) {
    FilePath = FileDevicePath(FileSystemHandles[Handle], AppPath);
    Status =
        gBS->LoadImage(TRUE, ImageHandle, FilePath, NULL, 0, &LoadedAppHandle);

    if (EFI_ERROR(Status) && FallbackPath != NULL) {
      FilePath = FileDevicePath(FileSystemHandles[Handle], FallbackPath);
      Status   = gBS->LoadImage(
          TRUE, ImageHandle, FilePath, NULL, 0, &LoadedAppHandle);
    }

    if (EFI_ERROR(Status)) {
      continue;
    }

    Status = gBS->HandleProtocol(
        LoadedAppHandle, &gEfiLoadedImageProtocolGuid, (VOID *)&LoadedAppImage);

    if (!EFI_ERROR(Status)) {
      Status = gBS->StartImage(LoadedAppHandle, NULL, NULL);
      return Status;
    }
  }

  return Status;
}

VOID EFIAPI DrawPrompt(BOOLEAN Clear)
{
  VOID *     ImageData;
  UINTN      ImageSize;
  EFI_STATUS Status;

  unsigned char *DecodedData;
  unsigned int   Width;
  unsigned int   Height;
  UINT32         DecoderError;

  if (Clear) {
    Status = GetSectionFromAnyFv(
        &gSplashPromptClearGuid, EFI_SECTION_RAW, 0, &ImageData, &ImageSize);
  }
  else {
    Status = GetSectionFromAnyFv(
        &gSplashPromptGuid, EFI_SECTION_RAW, 0, &ImageData, &ImageSize);
  }

  if (!EFI_ERROR(Status)) {
    DecoderError = lodepng_decode32(
        &DecodedData, &Width, &Height, (unsigned char *)ImageData,
        (size_t)ImageSize);

    if (!DecoderError) {
      ConvertBetweenBGRAandRGBA(DecodedData, Width, Height);

      // Just check for safety
      if (mGop->Mode->Info->VerticalResolution - Height > 0) {
        mGop->Blt(
            mGop, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)DecodedData,
            EfiBltBufferToVideo, 0, 0, 0,
            mGop->Mode->Info->VerticalResolution - Height, Width, Height, 0);
      }
    }
  }
}

EFI_STATUS EFIAPI InitializeBootSplash(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS                   Status;
  EFI_BOOT_MANAGER_LOAD_OPTION BootManagerMenu;
  UINTN                        Index;

  mCounterThreshold =
      EFI_TIMER_PERIOD_SECONDS(PcdGet16(PcdPlatformBootTimeOut)) /
      EFI_TIMER_PERIOD_MILLISECONDS(250);

  Status = gBS->LocateProtocol(
      &gEfiAbsolutePointerProtocolGuid, NULL, (VOID **)&mAbsPointerProtocol);
  if (EFI_ERROR(Status)) {
    goto exit;
  }

  Status = gBS->LocateProtocol(
      &gEfiSimpleTextInputExProtocolGuid, NULL,
      (VOID **)&mSimpleTextInputProtocol);
  if (EFI_ERROR(Status)) {
    goto exit;
  }

  Status = gBS->LocateProtocol(
      &gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&mGop);
  if (EFI_ERROR(Status)) {
    goto exit;
  }

  DrawPrompt(FALSE);

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL | EVT_TIMER, TPL_CALLBACK, InputPollingCallback, NULL,
      &mPollingEvent);
  ASSERT_EFI_ERROR(Status);

  Status = gBS->CreateEvent(0, 0, NULL, NULL, &mWaitingEvent);
  ASSERT_EFI_ERROR(Status);

  Status = gBS->SetTimer(
      mPollingEvent, TimerPeriodic, EFI_TIMER_PERIOD_MILLISECONDS(250));
  ASSERT_EFI_ERROR(Status);

  Status = gBS->WaitForEvent(1, &mWaitingEvent, &Index);
  ASSERT_EFI_ERROR(Status);

  // Polling event is already closed
  gBS->CloseEvent(&mWaitingEvent);

  // Clear the prompt
  DrawPrompt(TRUE);

  // Determine the result
  if (mBootTorEFIndApp) {
    DEBUG((EFI_D_INFO, "Booting into menu application \n"));
    Status = DiscoverAndBootApp(
        ImageHandle, REFIND_INSTALLATION_PATH,
        REFIND_FALLBACK_INSTALLATION_PATH);
  }

  Status = DiscoverAndBootApp(
      ImageHandle, EFI_REMOVABLE_MEDIA_FILE_NAME_AARCH64, NULL);

  // This is not expected, but it fails or exits, load shell/built-in menu
  Status = EfiBootManagerGetBootManagerMenu(&BootManagerMenu);
  if (!EFI_ERROR(Status)) {
    EfiBootManagerBoot(&BootManagerMenu);
  }
  else {
    gRT->ResetSystem(EfiResetWarm, Status, 0, NULL);
  }

exit:
  // Should not reach here
  return Status;
}
