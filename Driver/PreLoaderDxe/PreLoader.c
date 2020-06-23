#include <PiDxe.h>
#include <Uefi.h>

#include <Configuration/Hob.h>
#include <Pi/PiFirmwareFile.h>

#include <Library/BaseLib.h>
#include <Library/BmpSupportLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/EfiResetSystemLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/Bmp.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/GraphicsOutput.h>

#include <Library/BgraRgbaConvert.h>
#include <Library/lodepng.h>
#include <LittleVgl/core/lv_core/lv_refr.h>
#include <LittleVgl/core/lvgl.h>
#include <LittleVgl/lv_conf.h>

#include "AcpiTableInstaller.h"

STATIC EFI_GUID gUnsupportedImageGuid = {
    0x5fdf5e3c,
    0x6b70,
    0x4acc,
    {
        0x83,
        0x30,
        0x63,
        0xa6,
        0x73,
        0x92,
        0x4a,
        0x46,
    },
};

extern EFI_GUID gPreLoaderProtocolGuid;

EFI_GRAPHICS_OUTPUT_PROTOCOL *mGop;
lv_disp_drv_t                 mDispDrv;
lv_indev_drv_t                mFakeInputDrv;

static void EfiGopBltFlush(
    int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *color_p)
{
  mGop->Blt(
      mGop, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)color_p, EfiBltBufferToVideo, 0, 0,
      x1, y1, x2 - x1 + 1, y2 - y1 + 1, 0);

  lv_flush_ready();
}

static bool FakeInputRead(lv_indev_data_t *data) { return false; }

VOID EFIAPI BootShimVersionCheckFail(VOID)
{
  EFI_STATUS Status;
  VOID *     ImageData;
  UINTN      ImageSize;

  lv_img_dsc_t   png_dsc;
  unsigned char *DecodedData;
  unsigned int   Width;
  unsigned int   Height;
  UINT32         DecoderError;

  Status = GetSectionFromAnyFv(
      &gUnsupportedImageGuid, EFI_SECTION_RAW, 0, &ImageData, &ImageSize);
  ASSERT_EFI_ERROR(Status);

  // Prepare LittleVGL
  lv_init();

  lv_disp_drv_init(&mDispDrv);
  mDispDrv.disp_flush = EfiGopBltFlush;
  lv_disp_drv_register(&mDispDrv);

  lv_indev_drv_init(&mFakeInputDrv);
  mFakeInputDrv.type = LV_INDEV_TYPE_POINTER;
  mFakeInputDrv.read = FakeInputRead;
  lv_indev_drv_register(&mFakeInputDrv);

  // Decode PNG
  DecoderError = lodepng_decode32(
      &DecodedData, &Width, &Height, (unsigned char *)ImageData,
      (size_t)ImageSize);
  if (!DecoderError) {
    ConvertBetweenBGRAandRGBA(DecodedData, Width, Height);
    png_dsc.header.always_zero = 0;                 /*It must be zero*/
    png_dsc.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA; /*Set the color format*/
    png_dsc.header.w  = Width;
    png_dsc.header.h  = Height;
    png_dsc.data_size = Width * Height * 4;
    png_dsc.data      = DecodedData;

    // Present image
    lv_obj_t *img1 = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img1, &png_dsc);
    lv_obj_set_size(img1, 1080, 1920);
    lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 0);
  }

  // Refresh screen
  while (TRUE) {
    lv_tick_inc(1);
    lv_task_handler();
    gBS->Stall(EFI_TIMER_PERIOD_MILLISECONDS(1));
  }
}

EFI_STATUS
EFIAPI
PreLoaderDxeInitialize(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  PPRELOADER_ENVIRONMENT Env    = (VOID *)PRELOADER_ENV_ADDR;
  EFI_STATUS             Status = EFI_SUCCESS;

  // Protocols
  Status = gBS->LocateProtocol(
      &gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&mGop);
  ASSERT_EFI_ERROR(Status);

  // Library init
  Status = LibInitializeResetSystem(ImageHandle, SystemTable);
  ASSERT_EFI_ERROR(Status);

  // Version requirement
  if (Env->Header != PRELOADER_HEADER ||
      Env->PreloaderVersion < PRELOADER_VERSION_MIN) {
    BootShimVersionCheckFail();
    Status = EFI_UNSUPPORTED;
    goto exit;
  }

  // Install variables
  Status = gRT->SetVariable(
      L"UEFIDisplayInfo", &gEfiGraphicsOutputProtocolGuid,
      EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
      sizeof(UINT32) * 30, (VOID *)Env->UefiDisplayInfo);
  ASSERT_EFI_ERROR(Status);

  // Install ACPI Tables
  Status = InstallAcpiTables();
  ASSERT_EFI_ERROR(Status);

  // Install protocol
  Status = gBS->InstallProtocolInterface(
      &ImageHandle, &gPreLoaderProtocolGuid, EFI_NATIVE_INTERFACE,
      (void *)&ImageHandle);
  ASSERT_EFI_ERROR(Status);

exit:
  return Status;
}
