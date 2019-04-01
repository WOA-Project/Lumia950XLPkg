#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/MallocLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Library/RuntimeSpmiLib.h>
#include <Protocol/QcomSpmi.h>

#include "spmi_p.h"

#define R_SPMI_BASE ((UINTN)PcdGet64(PcdSpmiBaseAddress))

#define PMIC_ARB_V2 0x20010000
#define CHNL_IDX(sid, pid) ((sid << 8) | pid)

static uint32_t pmic_arb_chnl_num;
static uint32_t pmic_arb_owner_id;
static uint32_t pmic_arb_ver;
static uint32_t max_peripherals;
static uint32_t spmi_version = 0;

EFI_EVENT mRuntimeSpmiVirtualAddressChangedEvent;
STATIC UINTN pSpmiBaseAddressVirtual;

#define SPMI_BASE pSpmiBaseAddressVirtual

VOID EFIAPI RtSpmiConvertPointers(IN EFI_EVENT Event, IN VOID *Context)
{
  gRT->ConvertPointer(0, (VOID **)&pSpmiBaseAddressVirtual);
}

EFI_STATUS
EFIAPI
RtSpmiInitialize(VOID)
{
  EFI_STATUS Status = EFI_SUCCESS;

  spmi_version = PcdGet64(PcdSpmiVersion);
  if (spmi_version >= 2) {
    return EFI_UNSUPPORTED;
  }

  if (R_SPMI_BASE == 0) {
    DEBUG((EFI_D_ERROR, "PcdSpmiBaseAddress is required for SPMI\n"));
    return EFI_DEVICE_ERROR;
  }

  // Set address
  pSpmiBaseAddressVirtual = R_SPMI_BASE;

  /* Read the version number */
  pmic_arb_ver    = readl_rt(PMIC_ARB_SPMI_HW_VERSION);
  max_peripherals = PcdGet64(PcdSpmiMaxPeripherals);

  if (pmic_arb_ver < PMIC_ARB_V2) {
    /* Initialize PMIC Arbiter Channel Number to
     * 0 by default of V1 HW
     */
    pmic_arb_chnl_num = PcdGet64(PcdPmicArbChannelNum);
    pmic_arb_owner_id = PcdGet64(PcdPmicArbOwnerId);
  }
  else {
    return EFI_UNSUPPORTED;
  }

  // Register BS event for virtual address change
  Status = gBS->CreateEventEx(
      EVT_NOTIFY_SIGNAL, TPL_NOTIFY, RtSpmiConvertPointers, NULL,
      &gEfiEventVirtualAddressChangeGuid,
      &mRuntimeSpmiVirtualAddressChangedEvent);

  return Status;
}

STATIC
VOID EFIAPI RtSpmiWriteWDataFromArray(
    UINT8 *array, UINT8 reg_num, UINT8 array_size, UINT8 *bytes_written)
{
  uint32_t shift_value[] = {0, 8, 16, 24};
  int      i;
  uint32_t val = 0;

  /* Write to WDATA */
  for (i = 0; (*bytes_written < array_size) && (i < 4); i++) {
    val |= (uint32_t)(array[*bytes_written]) << shift_value[i];
    (*bytes_written)++;
  }

  writel_rt(val, PMICV1_ARB_CHNLn_WDATA(pmic_arb_chnl_num, reg_num));
}

UINTN
EFIAPI
RtSpmiPmicArbWriteCmd(PRUNTIME_PMIC_ARB_CMD cmd, PRUNTIME_PMIC_ARB_PARAM param)
{
  uint32_t bytes_written = 0;
  uint32_t error;
  uint32_t val = 0;

  /* Disable IRQ mode for the current channel*/
  writel_rt(0x0, PMICV1_ARB_CHNLn_CONFIG(pmic_arb_chnl_num));

  /* Write parameters for the cmd */
  if (cmd == NULL)
    return 1;

  /*
   * Write the data bytes according to the param->size
   * Can write upto 8 bytes.
   */

  /* Write first 4 bytes to WDATA0 */
  RtSpmiWriteWDataFromArray(
      param->buffer, 0, param->size, (uint8_t *)&bytes_written);

  if (bytes_written < param->size) {
    /* Write next 4 bytes to WDATA1 */
    RtSpmiWriteWDataFromArray(
        param->buffer, 1, param->size, (uint8_t *)&bytes_written);
  }

  /* Fill in the byte count for the command
   * Note: Byte count is one less than the number of bytes transferred.
   */
  cmd->byte_cnt = param->size - 1;
  /* Fill in the Write cmd opcode. */
  cmd->opcode = SPMI_CMD_EXT_REG_WRTIE_LONG;

  /* Write the command */
  val = 0;
  val |= ((uint32_t)(cmd->opcode) << PMIC_ARB_CMD_OPCODE_SHIFT);
  val |= ((uint32_t)(cmd->priority) << PMIC_ARB_CMD_PRIORITY_SHIFT);
  if (spmi_version != 2) {
    val |= ((uint32_t)(cmd->slave_id) << PMIC_ARB_CMD_SLAVE_ID_SHIFT);
    val |= ((uint32_t)(cmd->address) << PMIC_ARB_CMD_ADDR_SHIFT);
  }
  val |= ((uint32_t)(cmd->offset) << PMIC_ARB_CMD_ADDR_OFFSET_SHIFT);
  val |= ((uint32_t)(cmd->byte_cnt));

  writel_rt(val, PMICV1_ARB_CHNLn_CMD0(pmic_arb_chnl_num));

  /* Wait till CMD DONE status */
  while (!(val = readl_rt(PMICV1_ARB_CHNLn_STATUS(pmic_arb_chnl_num))))
    ;

  /* Check for errors */
  error = val ^ (1 << PMIC_ARB_CMD_DONE);
  if (error) {
    return error;
  }
  else {
    return 0;
  }
}
