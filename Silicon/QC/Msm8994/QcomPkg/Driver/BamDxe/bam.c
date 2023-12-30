/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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

#include <Library/LKEnvLib.h>

#include <Library/Pow2Lib.h>
#include <Protocol/QcomBam.h>

#include "bam_p.h"

#define HLOS_EE_INDEX 0

/* Reset BAM registers and pipes */
static void bam_reset(struct bam_instance *bam)
{
  /* Initiate SW reset */
  writel(BAM_SW_RST_BIT_MASK, BAM_CTRL_REG(bam->base));

  /* No delay required */

  /* Disable SW reset */
  writel(~BAM_SW_RST_BIT_MASK, BAM_CTRL_REG(bam->base));
}

/* Resets pipe registers and state machines */
static void bam_pipe_reset(struct bam_instance *bam, uint8_t pipe_num)
{
  /* Start sw reset of the pipe to be allocated */
  writel(1, BAM_P_RSTn(bam->pipe[pipe_num].pipe_num, bam->base));

  /* No delay required */

  /* Stop sw reset of the pipe to be allocated */
  writel(0, BAM_P_RSTn(bam->pipe[pipe_num].pipe_num, bam->base));
}

/* A blocking function that waits till an interrupt is signalled.
 * bam : BAM instance for the descriptors to be queued.
 * pipe_num : pipe number for the descriptors to be queued.
 * interrupt: interrupt to wait for.
 */
int bam_wait_for_interrupt(
    struct bam_instance *bam, uint8_t pipe_num, enum p_int_type interrupt)
{
  uint32_t val;

  while (1) {
    /* Wait for a interrupt on the right pipe */
    do {
      /* Determine the pipe causing the interrupt */
      val = readl(BAM_IRQ_SRCS(bam->base));
      /* Flush out the right most global interrupt bit */
    } while (!((val & 0x7FFF) & (1 << bam->pipe[pipe_num].pipe_num)));

    /* Check the reason for this BAM interrupt */
    if (readl(BAM_IRQ_STTS(bam->base)))
      goto bam_wait_int_error;

    /* Check the interrupt type */
    /* Read interrupt status register */
    val = readl(BAM_P_IRQ_STTSn(bam->pipe[pipe_num].pipe_num, bam->base));

    /* Check for error */
    if (val & P_ERR_EN_MASK)
      goto bam_wait_int_error;

    if (val & interrupt) {
      /* Correct interrupt was fired. */
      /* Clear the other interrupts */
      val = P_OUT_OF_DESC_EN_MASK | P_PRCSD_DESC_EN_MASK | P_TRNSFR_END_EN_MASK;
      writel(val, BAM_P_IRQ_CLRn(bam->pipe[pipe_num].pipe_num, bam->base));
      return BAM_RESULT_SUCCESS;
    }
    else if (val & P_TRNSFR_END_EN_MASK) {
      dprintf(
          CRITICAL,
          "Trasfer end signalled before the last descc was processed\n");
      goto bam_wait_int_error;
    }
  }

bam_wait_int_error:

  dprintf(CRITICAL, "Unexpected interrupt\n");
  return BAM_RESULT_FAILURE;
}

/* Enable BAM and pipe level interrupts */
void bam_enable_interrupts(struct bam_instance *bam, uint8_t pipe_num)
{

  uint32_t int_mask = P_ERR_EN_MASK | P_OUT_OF_DESC_EN_MASK |
                      P_PRCSD_DESC_EN_MASK | P_TRNSFR_END_EN_MASK;
  uint32_t val;

  /* Enable BAM error interrupts */
  writel(BAM_ERROR_EN_MASK, BAM_IRQ_EN_REG(bam->base));

  /* Enable the interrupts for the pipe by enabling the relevant bits
   * in the BAM_PIPE_INTERRUPT_ENABLE register.
   */
  writel(int_mask, BAM_P_IRQ_ENn(bam->pipe[pipe_num].pipe_num, bam->base));

  /* Enable pipe interrups */
  /* Do read-modify-write */
  val = readl(BAM_IRQ_SRCS_MSK(bam->base));
  writel(
      (1 << bam->pipe[pipe_num].pipe_num) | val, BAM_IRQ_SRCS_MSK(bam->base));
}

/* Reset and initialize the bam module */
void bam_init(struct bam_instance *bam)
{
  uint32_t val = 0;

  bam_reset(bam);

  /* Check for only one pipe's direction.
   * The other is assumed to be the opposite system
   * transaction.
   */
  if (bam->pipe[0].trans_type == SYS2BAM ||
      bam->pipe[0].trans_type == BAM2SYS) {
    /* Program the threshold count */
    writel(bam->threshold, BAM_DESC_CNT_TRSHLD_REG(bam->base));
  }

  /* Program config register for H/W bug fixes */
  val = 0xffffffff & ~(1 << 11);
  writel(val, BAM_CNFG_BITS(bam->base));

  /* Write the EE index to control the mapping of interrupts to EE */
  val = HLOS_EE_INDEX & BAM_EE_MASK;
  writel(val, BAM_TRUST_REG(bam->base));

  /* Enable the BAM */
  writel(BAM_ENABLE_BIT_MASK, BAM_CTRL_REG(bam->base));
}

/* Funtion to setup a simple fifo structure.
 * Note: Addr should be 8 byte aligned.
 * bam : BAM instance for the descriptors to be queued.
 * pipe_num : pipe number for the descriptors to be queued.
 */
int bam_pipe_fifo_init(struct bam_instance *bam, uint8_t pipe_num)
{
  if (bam->pipe[pipe_num].fifo.size > 0x7FFF) {
    dprintf(CRITICAL, "Size exceeds max size for a descriptor(0x7FFF)\n");
    return BAM_RESULT_FAILURE;
  }

  /* Check if fifo start is 8-byte alligned */
  ASSERT(!((uint32_t)bam->pipe[pipe_num].fifo.head & 0x7));

  /* Check if fifo size is a power of 2.
   * The circular fifo logic in lk expects this.
   */
  ASSERT(ispow2(bam->pipe[pipe_num].fifo.size));

  bam->pipe[pipe_num].fifo.current = bam->pipe[pipe_num].fifo.head;

  /* Set the descriptor buffer size. Must be a multiple of 8 */
  writel(
      bam->pipe[pipe_num].fifo.size * BAM_DESC_SIZE,
      BAM_P_FIFO_SIZESn(bam->pipe[pipe_num].pipe_num, bam->base));

  /* Write descriptors FIFO base addr must be 8-byte aligned */
  /* Needs a physical address conversion as we are setting up
   * the base of the FIFO for the BAM state machine.
   */
  writel(
      (uint32_t)bam->pipe[pipe_num].fifo.head,
      BAM_P_DESC_FIFO_ADDRn(bam->pipe[pipe_num].pipe_num, bam->base));

  /* Initialize FIFO offset for the first read */
  bam->pipe[pipe_num].fifo.offset = BAM_DESC_SIZE;

  /* Everything is set.
   * Flag pipe init done.
   */
  bam->pipe[pipe_num].initialized = 1;

  return BAM_RESULT_SUCCESS;
}

void bam_sys_pipe_init(struct bam_instance *bam, uint8_t pipe_num)
{
  uint32_t val = 0;

  /* Reset the pipe to be allocated */
  bam_pipe_reset(bam, pipe_num);

  /* Enable minimal interrupts */
  bam_enable_interrupts(bam, pipe_num);

  /* Pipe event threshold register is not relevant in sys modes */

  /* Enable pipe in system mode and set the direction */
  writel(
      P_SYS_MODE_MASK | P_ENABLE |
          (bam->pipe[pipe_num].trans_type << P_DIRECTION_SHIFT),
      BAM_P_CTRLn(bam->pipe[pipe_num].pipe_num, bam->base));

  /* Write the EE index to control the mapping of pipe interrupts to EE */
  val = HLOS_EE_INDEX & BAM_EE_MASK;
  writel(val, BAM_P_TRUST_REGn(bam->pipe[pipe_num].pipe_num, bam->base));

  /* Mark the pipe FIFO as uninitialized. */
  bam->pipe[pipe_num].initialized = 0;
}

/* Function to notify written descriptors to BAM.
 * bam : BAM instance for the descriptors to be queued.
 * pipe_num : pipe number for the descriptors to be queued.
 * num_desc : number of the descriptors.
 * fifo : Circular FIFO used for the descriptors.
 */
void bam_sys_gen_event(
    struct bam_instance *bam, uint8_t pipe_num, unsigned int num_desc)
{
  uint32_t val = 0;

  if (num_desc >= bam->pipe[pipe_num].fifo.size) {
    dprintf(CRITICAL, "Max allowed desc is one less than the fifo length\n");
    return;
  }

  /* Update the fifo peer offset */
  val = (num_desc - 1) * BAM_DESC_SIZE;
  val += bam->pipe[pipe_num].fifo.offset;
  val &= (bam->pipe[pipe_num].fifo.size * BAM_DESC_SIZE - 1);

  writel(val, BAM_P_EVNT_REGn(bam->pipe[pipe_num].pipe_num, bam->base));
}

/* Function to read the updates for FIFO offsets.
 * bam : BAM that uses the FIFO.
 * pipe : BAM pipe that uses the FIFO.
 * return : FIFO offset where the next descriptor should be written.
 * Note : S/W maintains the circular properties of the FIFO and updates
 *        the offsets accordingly.
 */
void bam_read_offset_update(struct bam_instance *bam, unsigned int pipe_num)
{
  uint32_t offset;

  offset = readl(BAM_P_SW_OFSTSn(bam->pipe[pipe_num].pipe_num, bam->base));
  offset &= 0xFFFF;

  dprintf(INFO, "Offset value is %d \n", offset);

  /* Save the next offset to be written to. */
  bam->pipe[pipe_num].fifo.current =
      (struct bam_desc *)((uint32_t)bam->pipe[pipe_num].fifo.head + offset);

  bam->pipe[pipe_num].fifo.offset = offset + BAM_DESC_SIZE;
}

/* Function to get the next desc address.
 * Keeps track of circular properties of the FIFO
 * and returns the appropriate address.
 */
static struct bam_desc *
fifo_getnext(struct bam_desc_fifo *fifo, struct bam_desc *desc)
{
  uint16_t offset;

  offset = desc - fifo->head;

  if (offset == (fifo->size - 1))
    return fifo->head;
  else
    return desc + 1;
}

/* Function to add BAM descriptors for a given fifo.
 * bam : BAM instance to be used.
 * data_ptr : Memory address for data transfer.
 * data_len : Length of the data_ptr.
 * flags : Flags to be set on the last desc added.
 *
 * Note: This function also notifies the BAM about the added descriptors.
 */
int bam_add_desc(
    struct bam_instance *bam, unsigned int pipe_num, unsigned char *data_ptr,
    unsigned int data_len, unsigned flags)
{
  int          bam_ret = BAM_RESULT_SUCCESS;
  unsigned int len     = data_len;
  unsigned int desc_len;
  unsigned int n = 0;
  unsigned int desc_flags;

  dprintf(INFO, "Data length for BAM transfer is %u\n", data_len);

  if (data_ptr == NULL || len == 0) {
    dprintf(CRITICAL, "Wrong params for BAM transfer \n");
    bam_ret = BAM_RESULT_FAILURE;
    goto bam_add_desc_error;
  }

  /* Check if we have enough space in FIFO */
  if (len > (unsigned)bam->pipe[pipe_num].fifo.size * BAM_MAX_DESC_DATA_LEN) {
    dprintf(CRITICAL, "Data transfer exceeds desc fifo length.\n");
    bam_ret = BAM_RESULT_FAILURE;
    goto bam_add_desc_error;
  }

  while (len) {

    /* There are only 16 bits to write data length.
     * If more bits are needed, create more
     * descriptors.
     */
    if (len > BAM_MAX_DESC_DATA_LEN) {
      desc_len = BAM_MAX_DESC_DATA_LEN;
      len -= BAM_MAX_DESC_DATA_LEN;
      desc_flags = 0;
    }
    else {
      desc_len = len;
      len      = 0;
      /* Set correct flags on the last desc. */
      desc_flags = flags;
    }

    /* Write descriptor */
    bam_add_one_desc(bam, pipe_num, data_ptr, desc_len, desc_flags);

    data_ptr += BAM_MAX_DESC_DATA_LEN;
    n++;
  }

  /* Create a read/write event to notify the periperal of the added desc. */
  bam_sys_gen_event(bam, pipe_num, n);

bam_add_desc_error:

  return bam_ret;
}

/* Function to add a BAM descriptor for a given fifo.
 * bam : BAM instance to be used.
 * data_ptr : Memory address for data transfer.
 * data_len : Length of the data_ptr.
 * flags : Flags to be set on the desc added.
 *
 * Note: This function does not notify the BAM about the added descriptor.
 */
int bam_add_one_desc(
    struct bam_instance *bam, unsigned int pipe_num, unsigned char *data_ptr,
    uint32_t len, uint8_t flags)
{

  struct bam_desc *desc    = bam->pipe[pipe_num].fifo.current;
  int              bam_ret = BAM_RESULT_SUCCESS;

  if (data_ptr == NULL || len == 0) {
    dprintf(CRITICAL, "Wrong params for BAM transfer \n");
    bam_ret = BAM_RESULT_FAILURE;
    goto bam_add_one_desc_error;
  }

  /* Check if the FIFO is allocated for the pipe */
  if (!bam->pipe[pipe_num].initialized) {
    dprintf(
        CRITICAL, "Please allocate the FIFO for the BAM pipe %d\n",
        bam->pipe[pipe_num].pipe_num);
    bam_ret = BAM_RESULT_FAILURE;
    goto bam_add_one_desc_error;
  }

  if ((flags & BAM_DESC_LOCK_FLAG) && (flags & BAM_DESC_UNLOCK_FLAG)) {
    dprintf(CRITICAL, "Can't lock and unlock in the same desc\n");
    bam_ret = BAM_RESULT_FAILURE;
    goto bam_add_one_desc_error;
  }

  /* Setting EOT flag on a CMD desc is not valid */
  if ((flags & BAM_DESC_EOT_FLAG) && (flags & BAM_DESC_CMD_FLAG)) {
    dprintf(CRITICAL, "EOT flag set on the CMD desc\n");
    bam_ret = BAM_RESULT_FAILURE;
    goto bam_add_one_desc_error;
  }

  /* Check for the length of the desc. */
  if (len > BAM_MAX_DESC_DATA_LEN) {
    dprintf(
        CRITICAL,
        "len of the desc exceeds max length"
        " %d > %d\n",
        len, BAM_MAX_DESC_DATA_LEN);
    bam_ret = BAM_RESULT_FAILURE;
    goto bam_add_one_desc_error;
  }

  desc->flags    = flags;
  desc->addr     = (uint32_t)data_ptr;
  desc->size     = (uint16_t)len;
  desc->reserved = 0;

  /* Update the FIFO to point to the head */
  bam->pipe[pipe_num].fifo.current =
      fifo_getnext(&bam->pipe[pipe_num].fifo, desc);

bam_add_one_desc_error:
  return bam_ret;
}

struct cmd_element *bam_add_cmd_element(
    struct cmd_element *ptr, uint32_t reg_addr, uint32_t value,
    enum bam_ce_cmd_t cmd_type)
{
  /* Write cmd type.
   * Also, write the register address.
   */
  ptr->addr_n_cmd = (reg_addr & ~(0xFF000000)) | (cmd_type << 24);

  /* Do not mask any of the addr bits by default */
  ptr->reg_mask = 0xFFFFFFFF;

  /* Write the value to be written */
  ptr->reg_data = value;

  /* Return the address to add the next element to */
  return ptr + 1;
}
