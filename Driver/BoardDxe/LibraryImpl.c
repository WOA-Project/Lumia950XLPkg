#include <Base.h>

#include <Library/LKEnvLib.h>
#include <Library/QcomBoardLib.h>

#include "board_p.h"

QCOM_BOARD_PROTOCOL *gBoard = NULL;

STATIC QCOM_BOARD_PROTOCOL mInternalBoard = {
    board_platform_id, board_target_id,         board_baseband,
    board_hardware_id, board_hardware_subtype,  board_foundry_id,
    board_chip_serial, board_chip_serial_char8, board_pmic_info,
    board_pmic_target, board_soc_version,       board_get_hw_platform_name,
};

RETURN_STATUS
EFIAPI
BoardImplLibInitialize(VOID)
{
  gBoard = &mInternalBoard;

  board_init();

  return RETURN_SUCCESS;
}
