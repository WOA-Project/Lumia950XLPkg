#include "../BootApp.h"

#include "../Modules.h"

EFI_ABSOLUTE_POINTER_PROTOCOL *mPointer;
lv_indev_drv_t                 mTouchInputDrv;

UINT64 LastX = 0;
UINT64 LastY = 0;

static bool EfiTouchScreenRead(lv_indev_data_t *data)
{
  if (mPointer == NULL || data == NULL)
    goto exit;

  EFI_ABSOLUTE_POINTER_STATE State;
  EFI_STATUS                 Status;

  Status = mPointer->GetState(mPointer, &State);
  if (EFI_ERROR(Status))
    goto exit;

  // Not changed
  if (State.CurrentX == LastX && State.CurrentY == LastY)
    goto exit;

  // Changed
  data->point.x =
      ((double)State.CurrentX / (double)mPointer->Mode->AbsoluteMaxX) *
      LV_HOR_RES;
  data->point.y =
      ((double)State.CurrentY / (double)mPointer->Mode->AbsoluteMaxY) *
      LV_VER_RES;
  data->state = LV_INDEV_STATE_PR;

  LastX = State.CurrentX;
  LastY = State.CurrentY;
  return true;

exit:
  return false;
}

EFIAPI EFI_STATUS EfiTouchScreenInit()
{
  EFI_STATUS Status = EFI_SUCCESS;
  Status            = gBS->LocateProtocol(
      &gEfiAbsolutePointerProtocolGuid, NULL, (VOID **)&mPointer);

  if (EFI_ERROR(Status))
    goto exit;
  ASSERT(mPointer != NULL);

  lv_indev_drv_init(&mTouchInputDrv);
  mTouchInputDrv.type = LV_INDEV_TYPE_POINTER;
  mTouchInputDrv.read = EfiTouchScreenRead;
  lv_indev_drv_register(&mTouchInputDrv);

exit:
  return Status;
}
