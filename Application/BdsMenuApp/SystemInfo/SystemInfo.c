#include "BootApp.h"

#include "../Modules.h"

EFIAPI EFI_STATUS SystemInfoEntry(lv_obj_t *tbSysInfo)
{
  EFI_STATUS Status = EFI_SUCCESS;
  CHAR8      TempStr8[1024];
  CHAR16     TempStr[1024];

  if (tbSysInfo == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  UnicodeSPrint(
      TempStr, sizeof(TempStr), L"Firmware release: %s %d.%02d \n",
      gST->FirmwareVendor, gST->FirmwareRevision >> 16,
      gST->FirmwareRevision & ((1 << 16) - 1));

  UnicodeStrToAsciiStrS(TempStr, TempStr8, sizeof(TempStr8));
  create_label_with_text(tbSysInfo, TempStr8);

  AsciiSPrint(
      TempStr8, sizeof(TempStr8), "Git commit: %a (%a) \n", __IMPL_COMMIT_ID__,
      __RELEASE_DATE__);
  create_label_with_text(tbSysInfo, TempStr8);

  AsciiSPrint(
      TempStr8, sizeof(TempStr8), "UEFI spec: %d.%02d \n",
      gST->Hdr.Revision >> 16, gST->Hdr.Revision & ((1 << 16) - 1));
  create_label_with_text(tbSysInfo, TempStr8);

  create_title_with_text(tbSysInfo, "Software Information \n\n");

  AsciiSPrint(
      TempStr8, sizeof(TempStr8), "Processor: %a \n",
      (CHAR8 *)FixedPcdGetPtr(PcdSmbiosProcessorModel));
  create_label_with_text(tbSysInfo, TempStr8);

  AsciiSPrint(
      TempStr8, sizeof(TempStr8), "System model: %a \n",
      (CHAR8 *)FixedPcdGetPtr(PcdSmbiosSystemModel));
  create_label_with_text(tbSysInfo, TempStr8);

  create_title_with_text(tbSysInfo, "Hardware Information \n\n");

exit:
  return Status;
}
