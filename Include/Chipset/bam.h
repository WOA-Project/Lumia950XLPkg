/* Copyright (c) 2012, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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

#ifndef __PLATFORM_MSM_SHARED_BAM_H
#define __PLATFORM_MSM_SHARED_BAM_H

#include <Library/LKEnvLib.h>

#define BAM_DESC_SIZE 8
#define BAM_CE_SIZE 16
#define BAM_MAX_DESC_DATA_LEN 0xFFFF
#define BAM_DATA_READ 0
#define BAM_DATA_WRITE 1

#define BAM_CTRL_REG(x) (0x0000 + (x))
#define BAM_SW_RST_BIT_MASK 1
#define BAM_ENABLE_BIT_MASK (1 << 1)

#define BAM_DESC_CNT_TRSHLD_REG(x) (0x0008 + (x))
#define COUNT_TRESHOLD_MASK 0xFF
#define BAM_IRQ_SRCS(x) (0x0000000C + (x))
#define BAM_IRQ_SRCS_MSK(x) (0x00000010 + (x))
#define BAM_IRQ_MASK (1 << 31)
#define P_IRQ_MASK (1)

/* Pipe Interrupt masks */
enum p_int_type {
  P_PRCSD_DESC_EN_MASK  = 1,
  P_OUT_OF_DESC_EN_MASK = (1 << 3),
  P_ERR_EN_MASK         = (1 << 4),
  P_TRNSFR_END_EN_MASK  = (1 << 5)
};

#define BAM_IRQ_STTS(x) (0x00000014 + (x))
#define BAM_IRQ_SRCS_UNMASKED(x) (0x00000030 + (x))

#define BAM_IRQ_EN_REG(x) (0x001C + (x))
#define BAM_TIMER_EN_MASK (1 << 4)
/* Available only in BAM-Lite */
#define BAM_EMPTY_EN_MASK (1 << 3)
#define BAM_ERROR_EN_MASK (1 << 2)
/* Available only in BAM */
#define BAM_HRESP_ERR_EN_MASK (1 << 1)

#define BAM_TRUST_REG(x) (0x00000070 + (x))
#define BAM_EE_MASK (7 << 0)
#define BAM_RESET_BLK_MASK (1 << 7)
#define BAM_LOCK_EE_CTRL_MASK (1 << 13)

#define BAM_CNFG_BITS(x) (0x0000007C + (x))

#define BAM_P_CTRLn(n, x) (0x00001000 + 0x1000 * (n) + (x))
#define P_SYS_MODE_MASK (1 << 5)
/* 1: Producer mode 0: Consumer mode */
#define P_DIRECTION_SHIFT 3
#define P_ENABLE (1 << 1)

#define BAM_P_RSTn(n, x) (0x00001000 + 0x4 + 0x1000 * (n) + (x))
#define BAM_P_IRQ_STTSn(n, x) (0x00001000 + 0x10 + 0x1000 * (n) + (x))
#define BAM_P_IRQ_CLRn(n, x) (0x00001000 + 0x14 + 0x1000 * (n) + (x))
#define BAM_P_IRQ_ENn(n, x) (0x00001000 + 0x18 + 0x1000 * (n) + (x))
#define BAM_P_TRUST_REGn(n, x) (0x00001000 + 0x30 + 0x1000 * (n) + (x))
#define BAM_P_SW_OFSTSn(n, x) (0x00001800 + 0x1000 * (n) + (x))
#define BAM_P_EVNT_REGn(n, x) (0x00001818 + 0x1000 * (n) + (x))
#define P_DESC_FIFO_PEER_OFST_MASK 0xFF

#define BAM_P_DESC_FIFO_ADDRn(n, x) (0x0000181C + 0x1000 * (n) + (x))
#define BAM_P_FIFO_SIZESn(n, x) (0x00001820 + 0x1000 * (n) + (x))

/* Flags for descriptors */
#define BAM_DESC_INT_FLAG (1 << 7)
#define BAM_DESC_EOT_FLAG (1 << 6)
#define BAM_DESC_EOB_FLAG (1 << 5)
#define BAM_DESC_NWD_FLAG (1 << 4)
#define BAM_DESC_CMD_FLAG (1 << 3)
#define BAM_DESC_LOCK_FLAG (1 << 2)
#define BAM_DESC_UNLOCK_FLAG (1 << 1)

enum bam_ce_cmd_t { CE_WRITE_TYPE = 0, CE_READ_TYPE = 1 };

/* result type */
typedef enum {
  BAM_RESULT_SUCCESS = 0,
  BAM_RESULT_FAILURE = 1,
  BAM_RESULT_TIMEOUT = 2
} bam_result_t;

/* Enum to define the BAM type:
 * BAM2BAM:Producer BAM to Consumer BAM.
 * SYS2BAM:Producer System to Consumer BAM.
 * BAM2SYS:Producer BAM to Consumer System.
 */
enum bam_transaction_type {
  SYS2BAM,
  BAM2SYS,
  BAM2BAM,
};

/* Enum to define BAM mode:
 * SPS:Use BAM pipes.
 * DIRECT:Pipes are disabled.
 * LEGACY:BAM is not used.
 */
enum bam_mode {
  SPS,
  DIRECT,
  LEGACY,
};

/* Enum to define BAM pipe states:
 * ENABLED:Producer and Consumer pipes are enabled.
 * HALT:Consumer pipe is halted. (Preferred type)
 * FULL_HALT:Both Producer and Consumer pipes are halted.
 */
enum bam_pipe_state {
  ENABLED,
  HALT,
  FULL_HALT,
};

enum bam_type {
  BAM_LITE,
  BAM,
};

/* Structure to define BAM descriptors that describe the data
 * descriptors written to the data FIFO.
 * addr:Descriptor address.
 * size:Each descriptor is 8 bytes. Size of the descriptor fifo must
 *      contain an integer number of Descriptors.
 */
struct bam_desc {
  uint32_t addr;
  uint16_t size;
  uint8_t  reserved;
  uint8_t  flags;
} __PACKED;

struct bam_desc_fifo {
  struct bam_desc *head;
  struct bam_desc *current;
  uint16_t         size;
  uint16_t         offset;
};

/* Structure to define BAM pipes
 * pipe_state: BAM pipe states.
 * trans_type: BAM tranaction type.
 * evt_gen_threshold: This register configures the threshold value for
 *                    Read/Write event generation by the BAM
 *                    towards another BAM.
 * fifo: Circular fifo associated with this pipe.
 * num_pipe: Number of pipes used in this bam.
 * pipe: Pipe number for this pipe.
 */
struct bam_pipe {
  enum bam_pipe_state       state;
  enum bam_transaction_type trans_type;
  struct bam_desc_fifo      fifo;
  uint16_t                  evt_gen_threshold;
  uint8_t                   pipe_num;
  uint8_t                   initialized;
};

/* Structure to define a BAM instance being used
 * base:Base address for the BAM.
 * type:BAM type.
 * mode:BAM mode.
 * pipe_pair:The pipe pairs to be used to access the BAM.
 * threshold:This Register holds a threshold value for the
 *           counter summing the Size of the Descriptors Provided.
 * init:Pipe initialization status for the BAM.
 */
struct bam_instance {
  uint32_t        base;
  enum bam_type   type;
  enum bam_mode   mode;
  uint8_t         num_of_pipes;
  struct bam_pipe pipe[3];
  uint16_t        threshold;
  void (*callback)(int);
};

/* Command element(CE) structure*/
struct cmd_element {
  uint32_t addr_n_cmd;
  uint32_t reg_data;
  uint32_t reg_mask;
  uint32_t reserve;
} __PACKED;

#endif
