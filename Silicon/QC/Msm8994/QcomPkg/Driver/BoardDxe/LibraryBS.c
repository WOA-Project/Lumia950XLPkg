#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomBoardLib.h>
#include <Library/UefiBootServicesTableLib.h>

QCOM_BOARD_PROTOCOL *gBoard = NULL;

RETURN_STATUS
EFIAPI
BoardLibConstructor(VOID)
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol(&gQcomBoardProtocolGuid, NULL, (VOID **)&gBoard);
  ASSERT_EFI_ERROR(Status);

  return Status;
}
