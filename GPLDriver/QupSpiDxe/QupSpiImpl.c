#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Chipset/gsbi.h>

#include <Library/ArmLib.h>
#include <Library/AtomicLib.h>
#include <Library/DebugLib.h>
#include <Library/InterruptsLib.h>
#include <Library/IoLib.h>
#include <Library/MallocLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/QcomClock.h>
#include <Protocol/QcomGpioTlmm.h>
#include <Protocol/QcomSpi.h>

#include "QupSpi.h"

extern QCOM_CLOCK_PROTOCOL *mClockProtocol;

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))
#define MAX_JIFFY_OFFSET ((LONG_MAX >> 1) - 1)
#define MSEC_PER_SEC 1000L
#define HZ 1000L

static inline unsigned long _msecs_to_jiffies(const unsigned int m)
{
  return (m + (MSEC_PER_SEC / HZ) - 1) / (MSEC_PER_SEC / HZ);
}

static inline unsigned long msecs_to_jiffies(const unsigned int m)
{
  if ((int)m < 0) {
    return MAX_JIFFY_OFFSET;
  }
  else {
    return _msecs_to_jiffies(m);
  }
}

static inline BOOLEAN spi_qup_is_valid_state(pspi_qup controller)
{
  uint32_t opstate = readl_relaxed(controller->base + QUP_STATE);
  return opstate & QUP_STATE_VALID;
}

EFI_STATUS spi_qup_set_state(pspi_qup controller, u32 state)
{
  unsigned long loop;
  u32           cur_state;
  loop = 0;

  while (!spi_qup_is_valid_state(controller)) {
    udelay(SPI_DELAY_THRESHOLD);
    if (++loop > SPI_DELAY_RETRY) {
      return EFI_DEVICE_ERROR;
    }
  }

  if (loop) {
    DEBUG(
        (EFI_D_ERROR, "Invalid state %d remained for %ld us but recovered\n",
         state, loop));
  }

  cur_state = readl_relaxed(controller->base + QUP_STATE);

  /*
   * Per spec: for PAUSE_STATE to RESET_STATE, two writes
   * of (b10) are required
   */
  if (((cur_state & QUP_STATE_MASK) == QUP_STATE_PAUSE) &&
      (state == QUP_STATE_RESET)) {
    writel_relaxed(QUP_STATE_CLEAR, controller->base + QUP_STATE);
    writel_relaxed(QUP_STATE_CLEAR, controller->base + QUP_STATE);
  }
  else {
    cur_state &= ~QUP_STATE_MASK;
    cur_state |= state;
    writel_relaxed(cur_state, controller->base + QUP_STATE);
  }
  loop = 0;
  while (!spi_qup_is_valid_state(controller)) {
    udelay(SPI_DELAY_THRESHOLD);
    if (++loop > SPI_DELAY_RETRY) {
      return EFI_DEVICE_ERROR;
    }
  }
  return EFI_SUCCESS;
}

void spi_qup_fifo_read(pspi_qup controller, struct spi_transfer *xfer)
{
  u8 *rx_buf = xfer->rx_buf;
  u32 word, state;
  int idx, shift, w_size;

  w_size = controller->w_size;
  while (controller->rx_bytes < xfer->len) {
    state = readl_relaxed(controller->base + QUP_OPERATIONAL);
    if (0 == (state & QUP_OP_IN_FIFO_NOT_EMPTY)) {
      break;
    }
    word = readl_relaxed(controller->base + QUP_INPUT_FIFO);
    if (!rx_buf) {
      controller->rx_bytes += w_size;
      continue;
    }
    for (idx = 0; idx < w_size; idx++, controller->rx_bytes++) {
      /*
       * The data format depends on bytes per SPI word:
       *  4 bytes: 0x12345678
       *  2 bytes: 0x00001234
       *  1 byte : 0x00000012
       */
      shift = BITS_PER_BYTE;
      shift *= (w_size - idx - 1);
      rx_buf[controller->rx_bytes] = word >> shift;
    }
  }
}

void spi_qup_fifo_write(pspi_qup controller, struct spi_transfer *xfer)
{
  const u8 *tx_buf = xfer->tx_buf;
  u32       word, state, data;
  int       idx, w_size;

  w_size = controller->w_size;
  while (controller->tx_bytes < xfer->len) {
    state = readl_relaxed(controller->base + QUP_OPERATIONAL);
    if (state & QUP_OP_OUT_FIFO_FULL) {
      break;
    }
    word = 0;
    for (idx = 0; idx < w_size; idx++, controller->tx_bytes++) {
      if (!tx_buf) {
        controller->tx_bytes += w_size;
        break;
      }
      data = tx_buf[controller->tx_bytes];
      word |= data << (BITS_PER_BYTE * (3 - idx));
    }
    writel_relaxed(word, controller->base + QUP_OUTPUT_FIFO);
  }
}

irqreturn_t qup_spi_interrupt(void *pdata)
{
  pspi_qup             controller = (pspi_qup)pdata;
  struct spi_transfer *xfer;

  u32        opflags, qup_err, spi_err;
  EFI_STATUS error = EFI_SUCCESS;

  xfer             = controller->xfer;
  controller->xfer = NULL;

  qup_err = readl_relaxed(controller->base + QUP_ERROR_FLAGS);
  spi_err = readl_relaxed(controller->base + SPI_ERROR_FLAGS);
  opflags = readl_relaxed(controller->base + QUP_OPERATIONAL);

  writel_relaxed(qup_err, controller->base + QUP_ERROR_FLAGS);
  writel_relaxed(spi_err, controller->base + SPI_ERROR_FLAGS);
  writel_relaxed(opflags, controller->base + QUP_OPERATIONAL);

  if (!xfer) {
    DEBUG(
        (EFI_D_ERROR, "unexpected irq %08x %08x %08x\n", qup_err, spi_err,
         opflags));
    return IRQ_NONE;
  }

  if (qup_err) {
    if (qup_err & QUP_ERROR_OUTPUT_OVER_RUN)
      DEBUG((EFI_D_ERROR, "OUTPUT_OVER_RUN\n"));
    if (qup_err & QUP_ERROR_INPUT_UNDER_RUN)
      DEBUG((EFI_D_ERROR, "INPUT_UNDER_RUN\n"));
    if (qup_err & QUP_ERROR_OUTPUT_UNDER_RUN)
      DEBUG((EFI_D_ERROR, "OUTPUT_UNDER_RUN\n"));
    if (qup_err & QUP_ERROR_INPUT_OVER_RUN)
      DEBUG((EFI_D_ERROR, "INPUT_OVER_RUN\n"));
    error = EFI_DEVICE_ERROR;
  }

  if (spi_err) {
    if (spi_err & SPI_ERROR_CLK_OVER_RUN)
      DEBUG((EFI_D_ERROR, "CLK_OVER_RUN\n"));
    if (spi_err & SPI_ERROR_CLK_UNDER_RUN)
      DEBUG((EFI_D_ERROR, "CLK_UNDER_RUN\n"));
    error = EFI_DEVICE_ERROR;
  }

  if (opflags & QUP_OP_IN_SERVICE_FLAG)
    spi_qup_fifo_read(controller, xfer);
  if (opflags & QUP_OP_OUT_SERVICE_FLAG)
    spi_qup_fifo_write(controller, xfer);

  controller->error = error;
  controller->xfer  = xfer;

  if (controller->rx_bytes == xfer->len || EFI_ERROR(error)) {
    AtomicSet(&controller->done, 1);
  }

  return IRQ_HANDLED;
}

EFI_STATUS
spi_qup_io_config(pspi_qup controller, struct spi_transfer *xfer)
{
  u32 config, iomode, mode;
  int ret, n_words, w_size;

  if (controller->mode & SPI_LOOP && xfer->len > controller->in_fifo_sz) {
    DEBUG(
        (EFI_D_ERROR, "too big size for loopback %d > %d\n", xfer->len,
         controller->in_fifo_sz));
    return EFI_DEVICE_ERROR;
  }

  ret = mClockProtocol->clk_set_rate(controller->cclk, xfer->speed_hz);
  if (ret) {
    DEBUG((EFI_D_ERROR, "fail to set frequency %d", xfer->speed_hz));
    return EFI_DEVICE_ERROR;
  }

  if (EFI_ERROR(spi_qup_set_state(controller, QUP_STATE_RESET))) {
    DEBUG((EFI_D_ERROR, "cannot set RESET state\n"));
    return EFI_DEVICE_ERROR;
  }

  w_size = 4;
  if (xfer->bits_per_word <= 8)
    w_size = 1;
  else if (xfer->bits_per_word <= 16)
    w_size = 2;

  n_words            = xfer->len / w_size;
  controller->w_size = w_size;

  if (n_words <= (controller->in_fifo_sz / sizeof(u32))) {
    mode = QUP_IO_M_MODE_FIFO;
    writel_relaxed(n_words, controller->base + QUP_MX_READ_CNT);
    writel_relaxed(n_words, controller->base + QUP_MX_WRITE_CNT);
    /* must be zero for FIFO */
    writel_relaxed(0, controller->base + QUP_MX_INPUT_CNT);
    writel_relaxed(0, controller->base + QUP_MX_OUTPUT_CNT);
  }
  else {
    mode = QUP_IO_M_MODE_BLOCK;
    writel_relaxed(n_words, controller->base + QUP_MX_INPUT_CNT);
    writel_relaxed(n_words, controller->base + QUP_MX_OUTPUT_CNT);
    /* must be zero for BLOCK and BAM */
    writel_relaxed(0, controller->base + QUP_MX_READ_CNT);
    writel_relaxed(0, controller->base + QUP_MX_WRITE_CNT);
  }

  iomode = readl_relaxed(controller->base + QUP_IO_M_MODES);
  /* Set input and output transfer mode */
  iomode &= ~(QUP_IO_M_INPUT_MODE_MASK | QUP_IO_M_OUTPUT_MODE_MASK);
  iomode &= ~(QUP_IO_M_PACK_EN | QUP_IO_M_UNPACK_EN);
  iomode |= (mode << QUP_IO_M_OUTPUT_MODE_MASK_SHIFT);
  iomode |= (mode << QUP_IO_M_INPUT_MODE_MASK_SHIFT);

  writel_relaxed(iomode, controller->base + QUP_IO_M_MODES);
  config = readl_relaxed(controller->base + SPI_CONFIG);
  if (controller->mode & SPI_LOOP)
    config |= SPI_CONFIG_LOOPBACK;
  else
    config &= ~SPI_CONFIG_LOOPBACK;
  if (controller->mode & SPI_CPHA)
    config &= ~SPI_CONFIG_INPUT_FIRST;
  else
    config |= SPI_CONFIG_INPUT_FIRST;

  /*
   * HS_MODE improves signal stability for spi-clk high rates,
   * but is invalid in loop back mode.
   */
  if ((xfer->speed_hz >= SPI_HS_MIN_RATE) && !(controller->mode & SPI_LOOP))
    config |= SPI_CONFIG_HS_MODE;
  else
    config &= ~SPI_CONFIG_HS_MODE;

  writel_relaxed(config, controller->base + SPI_CONFIG);
  config = readl_relaxed(controller->base + QUP_CONFIG);
  config &= ~(QUP_CONFIG_NO_INPUT | QUP_CONFIG_NO_OUTPUT | QUP_CONFIG_N);
  config |= xfer->bits_per_word - 1;
  config |= QUP_CONFIG_SPI_MODE;
  writel_relaxed(config, controller->base + QUP_CONFIG);

  /* only write to OPERATIONAL_MASK when register is present */
  if (!controller->qup_v1) {
    writel_relaxed(0, controller->base + QUP_OPERATIONAL_MASK);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
spi_qup_transfer_one(pspi_qup controller, struct spi_transfer *xfer)
{
  unsigned long timeout, flags;
  EFI_STATUS    Status;
  UINT64        TimerTicks64;
  UINT64        SystemCounterVal;

  Status = spi_qup_io_config(controller, xfer);
  if (EFI_ERROR(Status)) {
    goto exit;
  }

  timeout = DIV_ROUND_UP(xfer->speed_hz, MSEC_PER_SEC);
  timeout = DIV_ROUND_UP(xfer->len * 8, timeout);
  timeout = 100 * msecs_to_jiffies(timeout);

  AtomicSet(&controller->done, 0);
  controller->xfer     = xfer;
  controller->error    = 0;
  controller->rx_bytes = 0;
  controller->tx_bytes = 0;

  Status = spi_qup_set_state(controller, QUP_STATE_RUN);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "cannot set RUN state\n"));
    goto exit;
  }

  Status = spi_qup_set_state(controller, QUP_STATE_PAUSE);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "cannot set PAUSE state\n"));
    goto exit;
  }

  spi_qup_fifo_write(controller, xfer);
  Status = spi_qup_set_state(controller, QUP_STATE_RUN);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "cannot set EXECUTE state\n"));
    goto exit;
  }

  // Wait for completion
  SystemCounterVal = GetPerformanceCounter();
  TimerTicks64     = timeout + SystemCounterVal;
  while (SystemCounterVal < TimerTicks64) {
    if (AtomicRead(&controller->done) != 0) {
      break;
    }
    SystemCounterVal = GetPerformanceCounter();
  }

  if (AtomicRead(&controller->done) != 1) {
    Status = EFI_TIMEOUT;
  }

exit:
  spi_qup_set_state(controller, QUP_STATE_RESET);
  controller->xfer = NULL;
  if (EFI_ERROR(Status)) {
    if (EFI_ERROR(controller->error)) {
      Status = controller->error;
    }
  }
  return Status;
}
