#include "BootApp.h"

#include "../Modules.h"

EFI_EVENT      mTimerEvent;
lv_indev_drv_t mSidebuttonInputDrv;

BOOLEAN mModeInitialized = FALSE;
BOOLEAN mExitSignaled    = FALSE;
UINT16  mScanCode        = 0;

static lv_res_t mbox_apply_action(lv_obj_t *mbox, const char *txt)
{
  return LV_RES_OK; /*Return OK if the message box is not deleted*/
}

static lv_res_t continue_bds_action(lv_obj_t *btn) { return LV_RES_OK; }

static lv_res_t power_off_action(lv_obj_t *btn)
{
  gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
  return LV_RES_OK;
}

static lv_res_t reset_action(lv_obj_t *btn)
{
  gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
  return LV_RES_OK;
}

static bool EfiKeyRead(lv_indev_data_t *data)
{
  if (mScanCode != 0 && data != NULL) {
    data->btn   = mScanCode;
    data->state = LV_INDEV_STATE_PR;

    mScanCode = 0;
    return true;
  }

  return false;
}

void create_title_with_text(lv_obj_t *parent, const char *text)
{
  if (parent == NULL || text == NULL)
    return;

  static lv_style_t style_txt;
  lv_style_copy(&style_txt, &lv_style_plain);
  style_txt.text.font         = &lv_font_dejavu_40;
  style_txt.text.letter_space = 2;
  style_txt.text.line_space   = 2;

  lv_obj_t *label = lv_label_create(parent, NULL);
  lv_obj_set_style(label, &style_txt);
  lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
  lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
  lv_obj_set_width(label, 900);
  lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_label_set_text(label, text);
}

void create_label_with_text(lv_obj_t *parent, const char *text)
{
  if (parent == NULL || text == NULL)
    return;

  static lv_style_t style_txt;
  lv_style_copy(&style_txt, &lv_style_plain);
  style_txt.text.font         = &lv_font_dejavu_20;
  style_txt.text.letter_space = 2;
  style_txt.text.line_space   = 2;

  lv_obj_t *label = lv_label_create(parent, NULL);
  lv_obj_set_style(label, &style_txt);
  lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
  lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
  lv_obj_set_width(label, 900);
  lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_label_set_text(label, text);
}

VOID EFIAPI TimerCallback(IN EFI_EVENT Event, IN VOID *Context)
{
  lv_tick_inc(10);
  lv_task_handler();
}

VOID EFIAPI DrawMenu(VOID)
{
  lv_theme_t *th = lv_theme_material_init(10, NULL);
  lv_theme_set_current(th);

  static lv_style_t style_sb;
  lv_style_copy(&style_sb, &lv_style_plain);
  style_sb.body.main_color   = LV_COLOR_BLACK;
  style_sb.body.grad_color   = LV_COLOR_BLACK;
  style_sb.body.border.color = LV_COLOR_WHITE;
  style_sb.body.border.width = 1;
  style_sb.body.border.opa   = LV_OPA_70;
  style_sb.body.radius       = LV_RADIUS_CIRCLE;
  style_sb.body.opa          = LV_OPA_60;

  /* Create a window */
  lv_obj_t *win = lv_win_create(lv_scr_act(), NULL);
  lv_win_set_title(win, "Lumia UEFI Menu");
  lv_win_set_style(win, LV_WIN_STYLE_SB, &style_sb);

  /* Add control button to the header */
  lv_win_add_btn(win, SYMBOL_CLOSE, continue_bds_action);
  lv_win_add_btn(win, SYMBOL_POWER, power_off_action);
  lv_win_add_btn(win, SYMBOL_REFRESH, reset_action);

  /* Tabview */
  lv_obj_t *tbvFunc;
  tbvFunc = lv_tabview_create(win, NULL);

  lv_obj_t *tbSysinfo        = lv_tabview_add_tab(tbvFunc, "System Info");
  lv_obj_t *tbDeviceSettings = lv_tabview_add_tab(tbvFunc, "Device Settings");
  lv_obj_t *tbBootDevice     = lv_tabview_add_tab(tbvFunc, "Boot Device");

  /* System Info */
  SystemInfoEntry(tbSysinfo);
}

EFI_STATUS
EFIAPI
InitializeUserInterface(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS    Status = EFI_SUCCESS;
  UINTN         WaitIndex;
  EFI_INPUT_KEY Key;

  // Notify setup enter
  REPORT_STATUS_CODE(
      EFI_PROGRESS_CODE, (EFI_SOFTWARE_DXE_BS_DRIVER | EFI_SW_PC_USER_SETUP));

  // Initialize devices
  EfiBootManagerConnectAll();
  EfiBootManagerRefreshAllBootOption();

  // Init LV Core
  lv_init();

  // Initialize Display
  Status = GopDisplayInit();
  if (EFI_ERROR(Status))
    goto exit;

  // Initialize Touch
  Status = EfiTouchScreenInit();
  if (EFI_ERROR(Status))
    goto exit;

  // Initialize Keypad
  lv_indev_drv_init(&mSidebuttonInputDrv);
  mSidebuttonInputDrv.type = LV_INDEV_TYPE_KEYPAD;
  mSidebuttonInputDrv.read = EfiKeyRead;
  lv_indev_drv_register(&mSidebuttonInputDrv);

  // Initialize Menu
  DrawMenu();

  // Report DXE dispatcher status
  REPORT_STATUS_CODE(
      EFI_PROGRESS_CODE, (EFI_SOFTWARE_DXE_BS_DRIVER | EFI_SW_PC_INPUT_WAIT));

  // Set timers
  Status = gBS->SetWatchdogTimer(0, 0, 0, NULL);
  ASSERT_EFI_ERROR(Status);

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL | EVT_TIMER, TPL_CALLBACK, TimerCallback, NULL,
      &mTimerEvent);
  ASSERT_EFI_ERROR(Status);

  Status = gBS->SetTimer(
      mTimerEvent, TimerPeriodic, EFI_TIMER_PERIOD_MILLISECONDS(10));
  ASSERT_EFI_ERROR(Status);

  // UI Input Begin
  while (!mExitSignaled) {
    Status = gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &WaitIndex);
    ASSERT_EFI_ERROR(Status);
    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    if (!EFI_ERROR(Status))
      mScanCode = Key.ScanCode;
  }

exit:
  return Status;
}
