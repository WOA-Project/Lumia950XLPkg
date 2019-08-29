#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Chipset/gsbi.h>

#include <Library/ArmLib.h>
#include <Library/AtomicLib.h>
#include <Library/DebugLib.h>
#include <Library/InterruptsLib.h>
#include <Library/IoLib.h>
#include <Library/MallocLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/QcomClock.h>
#include <Protocol/QcomGpioTlmm.h>
#include <Protocol/QcomSpi.h>

#include "QupSpi.h"

QCOM_CLOCK_PROTOCOL *mClockProtocol;
spi_qup              mController;

EFI_STATUS EFIAPI Transfer(struct spi_transfer *xfer)
{
  if (xfer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  return spi_qup_transfer_one(&mController, xfer);
}

QCOM_SPI_QUP_PROTOCOL mSpiQup = {
    Transfer,
};

EFI_STATUS EFIAPI
           QupSpiDxeEntry(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  int   ret, size;
  UINTN base = (UINTN)0xf9966000;
  u32   max_freq, iomode, num_cs;

  // Get required clock protocol
  Status = gBS->LocateProtocol(
      &gQcomClockProtocolGuid, NULL, (VOID **)&mClockProtocol);
  if (EFI_ERROR(Status)) {
    goto exit;
  }

  // Pretty much static now as we only need BLSP2 QUP4 SPI (ACPI SP10)
  mController.base      = base;
  mController.irq       = FixedPcdGet64(PcdGicSpiStart) + 104;
  mController.error     = EFI_SUCCESS;
  mController.iface_clk = mClockProtocol->clk_get("qupF9963000_ahb_iface_clk");
  mController.cclk   = mClockProtocol->clk_get("gcc_blsp2_qup4_spi_apps_clk");
  mController.qup_v1 = 0;
  mController.mode   = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH | SPI_LOOP;
  AtomicSet(&mController.done, 0);

  // Enable BLSP2 QUP4 SPI clocks
  ret = mClockProtocol->clk_get_set_enable("qupF9963000_ahb_iface_clk", 0, 1);
  if (ret != 0) {
    DEBUG((DEBUG_ERROR, "Failed to enable BLSP2 AHB clock\n"));
    Status = EFI_DEVICE_ERROR;
    goto exit;
  }

  ret = mClockProtocol->clk_enable(mController.cclk);
  if (ret != 0) {
    DEBUG((DEBUG_ERROR, "Failed to enable BLSP2 QUP4 SPI core clock\n"));
    Status = EFI_DEVICE_ERROR;
    goto exit;
  }

  iomode = readl_relaxed(base + QUP_IO_M_MODES);
  size   = QUP_IO_M_OUTPUT_BLOCK_SIZE(iomode);
  if (size) {
    mController.out_blk_sz = size * 16;
  }
  else {
    mController.out_blk_sz = 4;
  }

  size = QUP_IO_M_INPUT_BLOCK_SIZE(iomode);
  if (size) {
    mController.in_blk_sz = size * 16;
  }
  else {
    mController.in_blk_sz = 4;
  }

  size                    = QUP_IO_M_OUTPUT_FIFO_SIZE(iomode);
  mController.out_fifo_sz = mController.out_blk_sz * (2 << size);
  size                    = QUP_IO_M_INPUT_FIFO_SIZE(iomode);
  mController.in_fifo_sz  = mController.in_blk_sz * (2 << size);

  DEBUG(
      (EFI_D_INFO, "IN:block:%d, fifo:%d, OUT:block:%d, fifo:%d\n",
       mController.in_blk_sz, mController.in_fifo_sz, mController.out_blk_sz,
       mController.out_fifo_sz));
  writel_relaxed(1, base + QUP_SW_RESET);

  Status = spi_qup_set_state(&mController, QUP_STATE_RESET);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "cannot set RESET state\n"));
    goto exit;
  }

  writel_relaxed(0, base + QUP_OPERATIONAL);
  writel_relaxed(0, base + QUP_IO_M_MODES);
  if (!mController.qup_v1) {
    writel_relaxed(0, base + QUP_OPERATIONAL_MASK);
  }

  writel_relaxed(
      SPI_ERROR_CLK_UNDER_RUN | SPI_ERROR_CLK_OVER_RUN,
      base + SPI_ERROR_FLAGS_EN);

  /* if earlier version of the QUP, disable INPUT_OVERRUN */
  if (mController.qup_v1) {
    writel_relaxed(
        QUP_ERROR_OUTPUT_OVER_RUN | QUP_ERROR_INPUT_UNDER_RUN |
            QUP_ERROR_OUTPUT_UNDER_RUN,
        base + QUP_ERROR_FLAGS_EN);
  }

  writel_relaxed(0, base + SPI_CONFIG);
  writel_relaxed(SPI_IO_C_NO_TRI_STATE, base + SPI_IO_CONTROL);

  /* Register IRQ */
  register_int_handler(
      mController.irq, (int_handler)qup_spi_interrupt, &mController);
  unmask_interrupt(mController.irq);

  // Install protocol
  Status = gBS->InstallMultipleProtocolInterfaces(
      &ImageHandle, &gQcomSpiQupProtocolGuid, &mSpiQup, NULL);

exit:
  ASSERT_EFI_ERROR(Status);
  return Status;
}
