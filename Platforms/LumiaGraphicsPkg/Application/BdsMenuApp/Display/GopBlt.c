#include "../BootApp.h"

#include "../Modules.h"

EFI_GRAPHICS_OUTPUT_PROTOCOL *mGop;
lv_disp_drv_t                 mDispDrv;

static void EfiGopBltFlush(
    int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *color_p)
{
  mGop->Blt(
      mGop, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)color_p, EfiBltBufferToVideo, 0, 0,
      x1, y1, x2 - x1 + 1, y2 - y1 + 1, 0);

  lv_flush_ready();
}

EFIAPI EFI_STATUS GopDisplayInit()
{
  EFI_STATUS Status = EFI_SUCCESS;
  Status            = gBS->LocateProtocol(
      &gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&mGop);

  if (EFI_ERROR(Status))
    goto exit;
  ASSERT(mGop != NULL);

  lv_disp_drv_init(&mDispDrv);
  mDispDrv.disp_flush = EfiGopBltFlush;
  lv_disp_drv_register(&mDispDrv);

exit:
  return Status;
}
