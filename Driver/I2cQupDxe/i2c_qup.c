/* Copyright (c) 2010-2014, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *  * Neither the name of The Linux Foundation, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * QUP driver for Qualcomm MSM platforms
 *
 */

/* #define DEBUG_QUP */

#include <Library/LKEnvLib.h>

#include <Chipset/gsbi.h>
#include <Chipset/i2c_qup.h>

#include <Library/AtomicLib.h>
#include <Library/InterruptsLib.h>
#include <Library/MallocLib.h>

#include <Library/QcomClockLib.h>
#include <Library/QcomGpioTlmmLib.h>
#include <Library/QcomPlatformI2cQupLib.h>

#include "i2c_qup_p.h"

#define max_t(type, x, y)                                                      \
  ({                                                                           \
    type __max1 = (x);                                                         \
    type __max2 = (y);                                                         \
    __max1 > __max2 ? __max1 : __max2;                                         \
  })

/* QUP Registers */
enum {
  QUP_CONFIG             = 0x0,
  QUP_STATE              = 0x4,
  QUP_IO_MODE            = 0x8,
  QUP_SW_RESET           = 0xC,
  QUP_OPERATIONAL        = 0x18,
  QUP_ERROR_FLAGS        = 0x1C,
  QUP_ERROR_FLAGS_EN     = 0x20,
  QUP_MX_READ_CNT        = 0x208,
  QUP_MX_INPUT_CNT       = 0x200,
  QUP_MX_WR_CNT          = 0x100,
  QUP_OUT_DEBUG          = 0x108,
  QUP_OUT_FIFO_CNT       = 0x10C,
  QUP_OUT_FIFO_BASE      = 0x110,
  QUP_IN_READ_CUR        = 0x20C,
  QUP_IN_DEBUG           = 0x210,
  QUP_IN_FIFO_CNT        = 0x214,
  QUP_IN_FIFO_BASE       = 0x218,
  QUP_I2C_CLK_CTL        = 0x400,
  QUP_I2C_STATUS         = 0x404,
  QUP_I2C_MASTER_BUS_CLR = 0x40C,
};

/* QUP States and reset values */
enum {
  QUP_RESET_STATE       = 0,
  QUP_RUN_STATE         = 1U,
  QUP_STATE_MASK        = 3U,
  QUP_PAUSE_STATE       = 3U,
  QUP_STATE_VALID       = 1U << 2,
  QUP_I2C_MAST_GEN      = 1U << 4,
  QUP_OPERATIONAL_RESET = 0xFF0,
  QUP_I2C_STATUS_RESET  = 0xFFFFFC,
};

/* QUP OPERATIONAL FLAGS */
enum {
  QUP_OUT_SVC_FLAG  = 1U << 8,
  QUP_IN_SVC_FLAG   = 1U << 9,
  QUP_MX_INPUT_DONE = 1U << 11,
};

/* I2C mini core related values */
enum {
  I2C_MINI_CORE = 2U << 8,
  I2C_N_VAL     = 0xF,

};

/* Packing Unpacking words in FIFOs , and IO modes*/
enum {
  QUP_WR_BLK_MODE = 1U << 10,
  QUP_RD_BLK_MODE = 1U << 12,
  QUP_UNPACK_EN   = 1U << 14,
  QUP_PACK_EN     = 1U << 15,
};

/* QUP tags */
enum {
  QUP_OUT_NOP   = 0,
  QUP_OUT_START = 1U << 8,
  QUP_OUT_DATA  = 2U << 8,
  QUP_OUT_STOP  = 3U << 8,
  QUP_OUT_REC   = 4U << 8,
  QUP_IN_DATA   = 5U << 8,
  QUP_IN_STOP   = 6U << 8,
  QUP_IN_NACK   = 7U << 8,
};

/* Status, Error flags */
enum {
  I2C_STATUS_WR_BUFFER_FULL = 1U << 0,
  I2C_STATUS_BUS_ACTIVE     = 1U << 8,
  I2C_STATUS_BUS_MASTER     = 1U << 9,
  I2C_STATUS_ERROR_MASK     = 0x38000FC,
  QUP_I2C_NACK_FLAG         = 1U << 3,
  QUP_IN_NOT_EMPTY          = 1U << 5,
  QUP_STATUS_ERROR_FLAGS    = 0x7C,
};

/* Master status clock states */
enum {
  I2C_CLK_RESET_BUSIDLE_STATE = 0,
  I2C_CLK_FORCED_LOW_STATE    = 5,
};

#define QUP_MAX_CLK_STATE_RETRIES 300
#define I2C_STATUS_CLK_STATE 13
#define QUP_OUT_FIFO_NOT_EMPTY 0x10
#define I2C_QUP_MAX_BUS_RECOVERY_RETRY 10

/* Register:QUP_I2C_MASTER_CLK_CTL field setters */
#define QUP_I2C_SCL_NOISE_REJECTION(reg_val, noise_rej_val)                    \
  (((reg_val) & ~(0x3 << 24)) | (((noise_rej_val)&0x3) << 24))
#define QUP_I2C_SDA_NOISE_REJECTION(reg_val, noise_rej_val)                    \
  (((reg_val) & ~(0x3 << 26)) | (((noise_rej_val)&0x3) << 26))

#ifdef DEBUG_QUP
static void qup_print_status(struct qup_i2c_dev *dev)
{
  uint32_t val;
  val = readl(dev->qup_base + QUP_CONFIG);
  dprintf(INFO, "Qup config is :0x%x\n", val);
  val = readl(dev->qup_base + QUP_STATE);
  dprintf(INFO, "Qup state is :0x%x\n", val);
  val = readl(dev->qup_base + QUP_IO_MODE);
  dprintf(INFO, "Qup mode is :0x%x\n", val);
}
#else
static inline void qup_print_status(struct qup_i2c_dev *dev) {}
#endif

static irqreturn_t qup_i2c_interrupt(void *pdata)
{
  struct qup_i2c_dev *dev     = pdata;
  uint32_t            status  = 0;
  uint32_t            status1 = 0;
  uint32_t            op_flgs = 0;
  int                 err     = 0;

  if (AtomicRead(&dev->xfer_progress) != 1) {
    dprintf(CRITICAL, "irq:%d when PM suspended\n", dev->qup_irq);
    return IRQ_NONE;
  }

  status  = readl(dev->qup_base + QUP_I2C_STATUS);
  status1 = readl(dev->qup_base + QUP_ERROR_FLAGS);
  op_flgs = readl(dev->qup_base + QUP_OPERATIONAL);

  if (!dev->msg) {
    /* Clear Error interrupt if it's a level triggered interrupt*/
    if (dev->num_irqs == 1) {
      writel(QUP_RESET_STATE, dev->qup_base + QUP_STATE);
    }
    return IRQ_HANDLED;
  }

  if (status & I2C_STATUS_ERROR_MASK) {
    dprintf(
        CRITICAL, "QUP: I2C status flags :0x%x, irq:%d\n", status,
        dev->qup_irq);
    err = status;
    /* Clear Error interrupt if it's a level triggered interrupt*/
    if (dev->num_irqs == 1) {
      writel(QUP_RESET_STATE, dev->qup_base + QUP_STATE);
    }
    goto intr_done;
  }

  if (status1 & 0x7F) {
    dprintf(CRITICAL, "QUP: QUP status flags :0x%x\n", status1);
    err = -status1;
    /* Clear Error interrupt if it's a level triggered interrupt*/
    if (dev->num_irqs == 1) {
      writel(
          (status1 & QUP_STATUS_ERROR_FLAGS), dev->qup_base + QUP_ERROR_FLAGS);
    }
    goto intr_done;
  }

  if (op_flgs & QUP_OUT_SVC_FLAG) {
    writel(QUP_OUT_SVC_FLAG, dev->qup_base + QUP_OPERATIONAL);
  }
  if (dev->msg->flags == I2C_M_RD) {
    if ((op_flgs & QUP_MX_INPUT_DONE) || (op_flgs & QUP_IN_SVC_FLAG)) {
      writel(QUP_IN_SVC_FLAG, dev->qup_base + QUP_OPERATIONAL);
    }
    else
      return IRQ_HANDLED;
  }

intr_done:
  dprintf(
      SPEW, "QUP intr= %d, i2c status=0x%x, qup status = 0x%x\n", dev->qup_irq,
      status, status1);
  qup_print_status(dev);
  dev->err = err;
  return IRQ_HANDLED;
}

static int
qup_i2c_poll_state(struct qup_i2c_dev *dev, uint32_t req_state, bool only_valid)
{
  uint32_t retries = 0;

  dprintf(
      SPEW, "Polling for state:0x%x, or valid-only:%d\n", req_state,
      only_valid);

  while (retries != 2000) {
    uint32_t status = readl(dev->qup_base + QUP_STATE);

    /*
     * If only valid bit needs to be checked, requested state is
     * 'don't care'
     */
    if (status & QUP_STATE_VALID) {
      if (only_valid)
        return 0;
      else if ((req_state & QUP_I2C_MAST_GEN) && (status & QUP_I2C_MAST_GEN))
        return 0;
      else if ((status & QUP_STATE_MASK) == req_state)
        return 0;
    }
    if (retries++ == 1000)
      udelay(100);
  }
  return -ETIMEDOUT;
}

static int qup_update_state(struct qup_i2c_dev *dev, uint32_t state)
{
  if (qup_i2c_poll_state(dev, 0, true) != 0) {
    dprintf(CRITICAL, "Poll State failed     \n");
    return -EIO;
  }

  writel(state, dev->qup_base + QUP_STATE);
  if (qup_i2c_poll_state(dev, state, false) != 0) {
    dprintf(CRITICAL, "Poll State failed (Secondary)     \n");
    return -EIO;
  }
  return 0;
}

static int qup_i2c_poll_writeready(struct qup_i2c_dev *dev, int rem)
{
  uint32_t retries = 0;

  while (retries != 2000) {
    uint32_t status = readl(dev->qup_base + QUP_I2C_STATUS);

    if (!(status & I2C_STATUS_WR_BUFFER_FULL)) {
      if (((dev->msg->flags & I2C_M_RD) || (rem == 0)) &&
          !(status & I2C_STATUS_BUS_ACTIVE))
        return 0;
      else if ((dev->msg->flags == 0) && (rem > 0))
        return 0;
      else /* 1-bit delay before we check for bus busy */
        udelay(dev->one_bit_t);
    }
    if (retries++ == 1000) {
      /*
       * Wait for FIFO number of bytes to be absolutely sure
       * that I2C write state machine is not idle. Each byte
       * takes 9 clock cycles. (8 bits + 1 ack)
       */
      udelay(dev->one_bit_t * (dev->out_fifo_sz * 9));
    }
  }
  qup_print_status(dev);
  return -ETIMEDOUT;
}

static int qup_i2c_poll_clock_ready(struct qup_i2c_dev *dev)
{
  uint32_t retries = 0;
  uint32_t op_flgs = -1, clk_state = -1;

  /*
   * Wait for the clock state to transition to either IDLE or FORCED
   * LOW.  This will usually happen within one cycle of the i2c clock.
   */

  while (retries++ < QUP_MAX_CLK_STATE_RETRIES) {
    uint32_t status = readl(dev->qup_base + QUP_I2C_STATUS);
    clk_state       = (status >> I2C_STATUS_CLK_STATE) & 0x7;
    /* Read the operational register */
    op_flgs = readl(dev->qup_base + QUP_OPERATIONAL) & QUP_OUT_FIFO_NOT_EMPTY;

    /*
     * In very corner case when slave do clock stretching and
     * output fifo will have 1 block of data space empty at
     * the same time.  So i2c qup will get output service
     * interrupt and as it doesn't have more data to be written.
     * This can lead to issue where output fifo is not empty.
     */
    if (op_flgs == 0 && (clk_state == I2C_CLK_RESET_BUSIDLE_STATE ||
                         clk_state == I2C_CLK_FORCED_LOW_STATE)) {
      dprintf(SPEW, "clk_state 0x%x op_flgs [%x]\n", clk_state, op_flgs);
      return 0;
    }

    /* 1-bit delay before we check again */
    udelay(dev->one_bit_t);
  }

  dprintf(
      CRITICAL, "Error waiting for clk ready clk_state: 0x%x op_flgs: 0x%x\n",
      clk_state, op_flgs);
  return -ETIMEDOUT;
}

#ifdef DEBUG
static void
qup_verify_fifo(struct qup_i2c_dev *dev, uint32_t val, uint32_t addr, int rdwr)
{
  if (rdwr)
    dprintf(SPEW, "RD:Wrote 0x%x to out_ff:0x%x\n", val, addr);
  else
    dprintf(SPEW, "WR:Wrote 0x%x to out_ff:0x%x\n", val, addr);
}
#else
static inline void
qup_verify_fifo(struct qup_i2c_dev *dev, uint32_t val, uint32_t addr, int rdwr)
{
}
#endif

static void qup_issue_read(
    struct qup_i2c_dev *dev, struct i2c_msg *msg, int *idx, uint32_t carry_over)
{
  uint16_t addr = (msg->addr << 1) | 1;
  /* QUP limit 256 bytes per read. By HW design, 0 in the 8-bit field
   * is treated as 256 byte read.
   */
  uint16_t rd_len = ((dev->cnt == 256) ? 0 : dev->cnt);

  if (*idx % 4) {
    writel(
        carry_over | ((QUP_OUT_START | addr) << 16),
        dev->qup_base + QUP_OUT_FIFO_BASE); /* + (*idx-2)); */

    qup_verify_fifo(
        dev, carry_over | ((QUP_OUT_START | addr) << 16),
        (uint32_t)dev->qup_base + QUP_OUT_FIFO_BASE + (*idx - 2), 1);
    writel(
        (QUP_OUT_REC | rd_len),
        dev->qup_base + QUP_OUT_FIFO_BASE); /* + (*idx+2)); */

    qup_verify_fifo(
        dev, (QUP_OUT_REC | rd_len),
        (uint32_t)dev->qup_base + QUP_OUT_FIFO_BASE + (*idx + 2), 1);
  }
  else {
    writel(
        ((QUP_OUT_REC | rd_len) << 16) | QUP_OUT_START | addr,
        dev->qup_base + QUP_OUT_FIFO_BASE); /* + (*idx)); */

    qup_verify_fifo(
        dev, QUP_OUT_REC << 16 | rd_len << 16 | QUP_OUT_START | addr,
        (uint32_t)dev->qup_base + QUP_OUT_FIFO_BASE + (*idx), 1);
  }
  *idx += 4;
}

static void qup_issue_write(
    struct qup_i2c_dev *dev, struct i2c_msg *msg, int rem, int *idx,
    uint32_t *carry_over)
{
  int      entries    = dev->cnt;
  int      empty_sl   = dev->wr_sz - ((*idx) >> 1);
  int      i          = 0;
  uint32_t val        = 0;
  uint32_t last_entry = 0;
  uint16_t addr       = msg->addr << 1;

  if (dev->pos == 0) {
    if (*idx % 4) {
      writel(
          *carry_over | ((QUP_OUT_START | addr) << 16),
          dev->qup_base + QUP_OUT_FIFO_BASE);

      qup_verify_fifo(
          dev, *carry_over | QUP_OUT_START << 16 | addr << 16,
          (uint32_t)dev->qup_base + QUP_OUT_FIFO_BASE + (*idx) - 2, 0);
    }
    else
      val = QUP_OUT_START | addr;
    *idx += 2;
    i++;
    entries++;
  }
  else {
    /* Avoid setp time issue by adding 1 NOP when number of bytes
     * are more than FIFO/BLOCK size. setup time issue can't appear
     * otherwise since next byte to be written will always be ready
     */
    val = (QUP_OUT_NOP | 1);
    *idx += 2;
    i++;
    entries++;
  }
  if (entries > empty_sl)
    entries = empty_sl;

  for (; i < (entries - 1); i++) {
    if (*idx % 4) {
      writel(
          val | ((QUP_OUT_DATA | msg->buf[dev->pos]) << 16),
          dev->qup_base + QUP_OUT_FIFO_BASE);

      qup_verify_fifo(
          dev, val | QUP_OUT_DATA << 16 | msg->buf[dev->pos] << 16,
          (uint32_t)dev->qup_base + QUP_OUT_FIFO_BASE + (*idx) - 2, 0);
    }
    else
      val = QUP_OUT_DATA | msg->buf[dev->pos];
    (*idx) += 2;
    dev->pos++;
  }
  if (dev->pos < (msg->len - 1))
    last_entry = QUP_OUT_DATA;
  else if (rem > 1) /* not last array entry */
    last_entry = QUP_OUT_DATA;
  else
    last_entry = QUP_OUT_STOP;
  if ((*idx % 4) == 0) {
    /*
     * If read-start and read-command end up in different fifos, it
     * may result in extra-byte being read due to extra-read cycle.
     * Avoid that by inserting NOP as the last entry of fifo only
     * if write command(s) leave 1 space in fifo.
     */
    if (rem > 1) {
      struct i2c_msg *next = msg + 1;
      if (next->addr == msg->addr && (next->flags & I2C_M_RD) &&
          *idx == ((dev->wr_sz * 2) - 4)) {
        writel(
            ((last_entry | msg->buf[dev->pos]) | ((1 | QUP_OUT_NOP) << 16)),
            dev->qup_base + QUP_OUT_FIFO_BASE); /* + (*idx) - 2); */

        qup_verify_fifo(
            dev,
            ((last_entry | msg->buf[dev->pos]) | ((1 | QUP_OUT_NOP) << 16)),
            (uint32_t)dev->qup_base + QUP_OUT_FIFO_BASE + (*idx), 0);
        *idx += 2;
      }
      else if (
          (dev->pos == msg->len - 1) && *idx < (dev->wr_sz * 2) &&
          (next->addr != msg->addr)) {
        /* Last byte of an intermittent write */
        writel(
            (QUP_OUT_STOP | msg->buf[dev->pos]),
            dev->qup_base + QUP_OUT_FIFO_BASE);

        qup_verify_fifo(
            dev, QUP_OUT_STOP | msg->buf[dev->pos],
            (uint32_t)dev->qup_base + QUP_OUT_FIFO_BASE + (*idx), 0);
        *idx += 2;
      }
      else
        *carry_over = (last_entry | msg->buf[dev->pos]);
    }
    else {
      writel(
          (last_entry | msg->buf[dev->pos]),
          dev->qup_base + QUP_OUT_FIFO_BASE); /* + (*idx) - 2); */

      qup_verify_fifo(
          dev, last_entry | msg->buf[dev->pos],
          (uint32_t)dev->qup_base + QUP_OUT_FIFO_BASE + (*idx), 0);
    }
  }
  else {
    writel(
        val | ((last_entry | msg->buf[dev->pos]) << 16),
        dev->qup_base + QUP_OUT_FIFO_BASE); /* + (*idx) - 2); */

    qup_verify_fifo(
        dev, val | (last_entry << 16) | (msg->buf[dev->pos] << 16),
        (uint32_t)dev->qup_base + QUP_OUT_FIFO_BASE + (*idx) - 2, 0);
  }

  *idx += 2;
  dev->pos++;
  dev->cnt = msg->len - dev->pos;
}

static void qup_set_read_mode(struct qup_i2c_dev *dev, int rd_len)
{
  uint32_t wr_mode = (dev->wr_sz < dev->out_fifo_sz) ? QUP_WR_BLK_MODE : 0;
  if (rd_len > 256) {
    dprintf(SPEW, "HW limit: Breaking reads in chunk of 256\n");
    rd_len = 256;
  }
  if (rd_len <= dev->in_fifo_sz) {
    writel(wr_mode | QUP_PACK_EN | QUP_UNPACK_EN, dev->qup_base + QUP_IO_MODE);
    writel(rd_len, dev->qup_base + QUP_MX_READ_CNT);
  }
  else {
    writel(
        wr_mode | QUP_RD_BLK_MODE | QUP_PACK_EN | QUP_UNPACK_EN,
        dev->qup_base + QUP_IO_MODE);
    writel(rd_len, dev->qup_base + QUP_MX_INPUT_CNT);
  }
}

static int qup_set_wr_mode(struct qup_i2c_dev *dev, int rem)
{
  int             total_len = 0;
  int             ret       = 0;
  int             len       = dev->msg->len;
  struct i2c_msg *next      = NULL;
  if (rem > 1)
    next = dev->msg + 1;
  while (rem > 1 && next->flags == 0 && (next->addr == dev->msg->addr)) {
    len += next->len + 1;
    next = next + 1;
    rem--;
  }
  if (len >= (dev->out_fifo_sz - 1)) {
    total_len = len + 1 + (len / (dev->out_blk_sz - 1));

    writel(
        QUP_WR_BLK_MODE | QUP_PACK_EN | QUP_UNPACK_EN,
        dev->qup_base + QUP_IO_MODE);
    dev->wr_sz = dev->out_blk_sz;
  }
  else
    writel(QUP_PACK_EN | QUP_UNPACK_EN, dev->qup_base + QUP_IO_MODE);

  if (rem > 1) {
    if (next->addr == dev->msg->addr && next->flags == I2C_M_RD) {
      qup_set_read_mode(dev, next->len);
      /* make sure read start & read command are in 1 blk */
      if ((total_len % dev->out_blk_sz) == (dev->out_blk_sz - 1))
        total_len += 3;
      else
        total_len += 2;
    }
  }
  /* WRITE COUNT register valid/used only in block mode */
  if (dev->wr_sz == dev->out_blk_sz)
    writel(total_len, dev->qup_base + QUP_MX_WR_CNT);
  return ret;
}

static int qup_i2c_reset(struct qup_i2c_dev *dev)
{
  int ret;

  /* sw reset */
  writel(1, dev->qup_base + QUP_SW_RESET);
  ret = qup_i2c_poll_state(dev, QUP_RESET_STATE, false);
  if (ret) {
    dprintf(CRITICAL, "QUP Busy:Trying to recover\n");
    return ret;
  }

  /* Initialize QUP registers */
  writel(0, dev->qup_base + QUP_CONFIG);
  writel(QUP_OPERATIONAL_RESET, dev->qup_base + QUP_OPERATIONAL);
  writel(QUP_STATUS_ERROR_FLAGS, dev->qup_base + QUP_ERROR_FLAGS_EN);

  writel(I2C_MINI_CORE | I2C_N_VAL, dev->qup_base + QUP_CONFIG);

  /* Initialize I2C mini core registers */
  writel(0, dev->qup_base + QUP_I2C_CLK_CTL);
  writel(QUP_I2C_STATUS_RESET, dev->qup_base + QUP_I2C_STATUS);

  return ret;
}

static int qup_i2c_try_recover_bus_busy(struct qup_i2c_dev *dev)
{
  int           ret;
  uint32_t      status;
  unsigned long min_sleep_usec;

  mask_interrupt(dev->qup_irq);

  qup_i2c_reset(dev);

  ret = qup_update_state(dev, QUP_RUN_STATE);
  if (ret < 0) {
    dprintf(CRITICAL, "error: bus clear fail to set run state\n");
    goto recovery_end;
  }

  writel(dev->clk_ctl, dev->qup_base + QUP_I2C_CLK_CTL);

  writel(0x1, dev->qup_base + QUP_I2C_MASTER_BUS_CLR);

  /*
   * wait for bus clear (9 clock pulse cycles) to complete.
   * min_time = 9 clock *10  (1000% margin)
   * max_time = 10* min_time
   */
  min_sleep_usec =
      max_t(unsigned long, (9 * 10 * USEC_PER_SEC) / dev->clk_freq, 100);
  udelay(min_sleep_usec);

  status = readl(dev->qup_base + QUP_I2C_STATUS);

recovery_end:
  unmask_interrupt(dev->qup_irq);
  return ret;
}

static void qup_i2c_recover_bus_busy(struct qup_i2c_dev *dev)
{
  uint32_t bus_clr, bus_active, status;
  int      retry = 0;
  dprintf(INFO, "Executing bus recovery procedure (9 clk pulse)\n");

  do {
    qup_i2c_try_recover_bus_busy(dev);
    bus_clr    = readl(dev->qup_base + QUP_I2C_MASTER_BUS_CLR);
    status     = readl(dev->qup_base + QUP_I2C_STATUS);
    bus_active = status & I2C_STATUS_BUS_ACTIVE;
    if (++retry >= I2C_QUP_MAX_BUS_RECOVERY_RETRY)
      break;
  } while (bus_clr || bus_active);

  dprintf(
      INFO, "Bus recovery %s after %d retries\n",
      (bus_clr || bus_active) ? "fail" : "success", retry);
}

int qup_i2c_xfer(struct qup_i2c_dev *dev, struct i2c_msg msgs[], int num)
{
  int ret;
  int rem = num;
  int err;

  if (dev->suspended) {
    return -EIO;
  }

  AtomicSet(&dev->xfer_progress, 1);
  /* Initialize QUP registers during first transfer */
  if (dev->clk_ctl == 0) {
    int      fs_div;
    int      hs_div;
    uint32_t fifo_reg;

    if (dev->device_type == I2C_QUP_DEVICE_TYPE_GSBI) {
      /* Configure the GSBI Protocol Code for i2c */
      writel(
          (GSBI_PROTOCOL_CODE_I2C << GSBI_CTRL_REG_PROTOCOL_CODE_S),
          GSBI_CTRL_REG(dev->gsbi_base));
    }

    fs_div       = ((dev->src_clk_rate / dev->clk_freq) / 2) - 3;
    hs_div       = 3;
    dev->clk_ctl = ((hs_div & 0x7) << 8) | (fs_div & 0xff);
    dev->clk_ctl =
        QUP_I2C_SCL_NOISE_REJECTION(dev->clk_ctl, dev->noise_rjct_scl);
    dev->clk_ctl =
        QUP_I2C_SDA_NOISE_REJECTION(dev->clk_ctl, dev->noise_rjct_sda);
    fifo_reg = readl(dev->qup_base + QUP_IO_MODE);
    if (fifo_reg & 0x3)
      dev->out_blk_sz = (fifo_reg & 0x3) * 16;
    else
      dev->out_blk_sz = 16;
    if (fifo_reg & 0x60)
      dev->in_blk_sz = ((fifo_reg & 0x60) >> 5) * 16;
    else
      dev->in_blk_sz = 16;
    /*
     * The block/fifo size w.r.t. 'actual data' is 1/2 due to 'tag'
     * associated with each byte written/received
     */
    dev->out_blk_sz /= 2;
    dev->in_blk_sz /= 2;
    dev->out_fifo_sz = dev->out_blk_sz * (2 << ((fifo_reg & 0x1C) >> 2));
    dev->in_fifo_sz  = dev->in_blk_sz * (2 << ((fifo_reg & 0x380) >> 7));
    dprintf(
        SPEW, "QUP IN:bl:%d, ff:%d, OUT:bl:%d, ff:%d\n", dev->in_blk_sz,
        dev->in_fifo_sz, dev->out_blk_sz, dev->out_fifo_sz);
  }

  if (qup_i2c_reset(dev))
    dprintf(CRITICAL, "warning: QUP reset before a xfer failed\n");

  unmask_interrupt(dev->qup_irq);

  while (rem) {
    bool filled = false;

    dev->cnt = msgs->len - dev->pos;
    dev->msg = msgs;

    dev->wr_sz = dev->out_fifo_sz;
    dev->err   = 0;

    if (qup_i2c_poll_state(dev, QUP_I2C_MAST_GEN, false) != 0) {
      dprintf(CRITICAL, "Poll State failed     \n");
      ret = -EIO;
      goto out_err;
    }

    qup_print_status(dev);
    /* HW limits Read upto 256 bytes in 1 read without stop */
    if (dev->msg->flags & I2C_M_RD) {
      qup_set_read_mode(dev, dev->cnt);
      if (dev->cnt > 256)
        dev->cnt = 256;
    }
    else {
      ret = qup_set_wr_mode(dev, rem);
      if (ret != 0)
        goto out_err;
      /* Don't fill block till we get interrupt */
      if (dev->wr_sz == dev->out_blk_sz)
        filled = true;
    }

    err = qup_update_state(dev, QUP_RUN_STATE);
    if (err < 0) {
      ret = err;
      goto out_err;
    }

    qup_print_status(dev);
    writel(dev->clk_ctl, dev->qup_base + QUP_I2C_CLK_CTL);

    do {
      int      idx        = 0;
      uint32_t carry_over = 0;

      /* Transition to PAUSE state only possible from RUN */
      err = qup_update_state(dev, QUP_PAUSE_STATE);
      if (err < 0) {
        ret = err;
        goto out_err;
      }

      qup_print_status(dev);
      /* This operation is Write, check the next operation
       * and decide mode
       */
      while (filled == false) {
        if ((msgs->flags & I2C_M_RD))
          qup_issue_read(dev, msgs, &idx, carry_over);
        else if (!(msgs->flags & I2C_M_RD))
          qup_issue_write(dev, msgs, rem, &idx, &carry_over);
        if (idx >= (dev->wr_sz << 1))
          filled = true;
        /* Start new message */
        if (filled == false) {
          if (msgs->flags & I2C_M_RD)
            filled = true;
          else if (rem > 1) {
            /* Only combine operations with
             * same address
             */
            struct i2c_msg *next = msgs + 1;
            if (next->addr != msgs->addr)
              filled = true;
            else {
              rem--;
              msgs++;
              dev->msg = msgs;
              dev->pos = 0;
              dev->cnt = msgs->len;
              if (msgs->len > 256)
                dev->cnt = 256;
            }
          }
          else
            filled = true;
        }
      }
      err = qup_update_state(dev, QUP_RUN_STATE);
      if (err < 0) {
        ret = err;
        goto out_err;
      }
      dprintf(
          SPEW, "idx:%d, rem:%d, num:%d, mode:%d\n", idx, rem, num, dev->mode);

      qup_print_status(dev);
      if (dev->err) {
        if (dev->err > 0 && dev->err & QUP_I2C_NACK_FLAG) {
          dprintf(
              CRITICAL, "I2C slave addr:0x%x not connected\n", dev->msg->addr);
          dev->err = ERR_IO;
        }
        else if (dev->err < 0) {
          dprintf(INFO, "QUP data xfer error %d\n", dev->err);
          ret = -ERR_IO;
          goto out_err;
        }
        else if (dev->err > 0) {
          /*
           * ISR returns +ve error if error code
           * is I2C related, e.g. unexpected start
           * So you may call recover-bus-busy when
           * this error happens
           */
          qup_i2c_recover_bus_busy(dev);
        }
        ret = -ERR_IO;
        goto out_err;
      }
      if (dev->msg->flags & I2C_M_RD) {
        int      i;
        uint32_t dval = 0;
        for (i = 0; dev->pos < dev->msg->len; i++, dev->pos++) {
          uint32_t rd_status = readl(dev->qup_base + QUP_OPERATIONAL);
          if (i % 2 == 0) {
            if ((rd_status & QUP_IN_NOT_EMPTY) == 0)
              break;
            dval                    = readl(dev->qup_base + QUP_IN_FIFO_BASE);
            dev->msg->buf[dev->pos] = dval & 0xFF;
          }
          else
            dev->msg->buf[dev->pos] = ((dval & 0xFF0000) >> 16);
        }
        dev->cnt -= i;
      }
      else
        filled = false; /* refill output FIFO */
      dprintf(SPEW, "pos:%d, len:%d, cnt:%d\n", dev->pos, msgs->len, dev->cnt);
    } while (dev->cnt > 0);
    if (dev->cnt == 0) {
      if (msgs->len == dev->pos) {
        rem--;
        msgs++;
        dev->pos = 0;
      }
      if (rem) {
        err = qup_i2c_poll_clock_ready(dev);
        if (err < 0) {
          ret = err;
          goto out_err;
        }
        err = qup_update_state(dev, QUP_RESET_STATE);
        if (err < 0) {
          ret = err;
          goto out_err;
        }
      }
    }
    /* Wait for I2C bus to be idle */
    ret = qup_i2c_poll_writeready(dev, rem);
    if (ret) {
      dprintf(ERROR, "Error waiting for write ready\n");
      goto out_err;
    }
  }

  ret = num;
out_err:
  mask_interrupt(dev->qup_irq);
  dev->msg = NULL;
  dev->pos = 0;
  dev->err = 0;
  dev->cnt = 0;
  AtomicSet(&dev->xfer_progress, 0);
  return ret;
}

void qup_i2c_sec_init(
    struct qup_i2c_dev *dev, uint32_t clk_freq, uint32_t src_clk_rate)
{
  /* Set clk_freq and src_clk_rate for i2c. */
  dev->clk_freq     = clk_freq;
  dev->src_clk_rate = src_clk_rate;

  dev->num_irqs = 1;

  dev->one_bit_t = USEC_PER_SEC / dev->clk_freq;
  dev->clk_ctl   = 0;

  AtomicSet(&dev->xfer_progress, 0);

  /* Register the GSBIn QUP IRQ */
  register_int_handler(dev->qup_irq, (int_handler)qup_i2c_interrupt, dev);

  /* Then disable it */
  mask_interrupt(dev->qup_irq);
}

int qup_i2c_deinit(struct qup_i2c_dev *dev)
{
  /* Disable the qup_irq */
  mask_interrupt(dev->qup_irq);
  /* Free the memory used for dev */
  free(dev);
  return 0;
}