/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    sdm845.h

Abstract:

    This header file declares shared values for the two Qualcomm SDM 845
    serial UARTs.

--*/

#pragma once

// ---------------------------------------------------------------- Definitions

#define in_dword_masked(Address, Mask) (MmioRead32(Address) & (Mask))
#define out_dword(Address, Data) MmioWrite32(Address, Data)
#define out_dword_masked_ns(Address, Mask, Data, PreviousData) out_dword(Address, (PreviousData & (UINT32)(~(Mask))) | ((UINT32)Data & (Mask)))

// Different base addresses offset
#define GENI_CFG            0x0
#define GENI_IMAGE_REGS     0x100
#define GENI_DATA           0x600
#define QUPV3_SE_DMA        0xC00
#define GENI_IMAGE          0x1000

//
// QUPV3_SE
//

#define TX_FIFO_DEPTH_MASK      HWIO_SE_HW_PARAM_0_TX_FIFO_DEPTH_BMSK
#define TX_FIFO_DEPTH_SHIFT     HWIO_SE_HW_PARAM_0_TX_FIFO_DEPTH_SHFT

#define RX_FIFO_DEPTH_MASK      HWIO_SE_HW_PARAM_1_RX_FIFO_DEPTH_BMSK
#define RX_FIFO_DEPTH_SHIFT     HWIO_SE_HW_PARAM_1_RX_FIFO_DEPTH_SHFT

//
// FIFO
//

#define RX_LAST_VALID_BYTES_MASK     HWIO_GENI_RX_FIFO_STATUS_RX_LAST_BYTE_VALID_BMSK
#define RX_LAST_VALID_BYTES_SHIFT    HWIO_GENI_RX_FIFO_STATUS_RX_LAST_BYTE_VALID_SHFT
#define RX_FIFO_WC                   HWIO_GENI_RX_FIFO_STATUS_RX_FIFO_WC_BMSK
#define RX_FIFO_WC_SHIFT             HWIO_GENI_RX_FIFO_STATUS_RX_FIFO_WC_SHFT

//
// IRQ
//

#define TF_FIFO_WATERMARK_IRQ       HWIO_GENI_M_IRQ_STATUS_TX_FIFO_WATERMARK_BMSK
#define M_CMD_DONE_IRQ              HWIO_GENI_M_IRQ_STATUS_M_CMD_DONE_BMSK
#define SEC_IRQ                     HWIO_GENI_M_IRQ_STATUS_SEC_IRQ_BMSK

#define RX_LAST_IRQ                 HWIO_GENI_S_IRQ_STATUS_RX_FIFO_LAST_BMSK
#define RX_FIFO_WATERMARK_IRQ       HWIO_GENI_S_IRQ_STATUS_RX_FIFO_WATERMARK_BMSK

//
// MODULE: GENI_CFG
//

#define HWIO_GENI_OUTPUT_CTRL_ADDR(x)                                                           ((x) + 0x00000024)
#define HWIO_GENI_OUTPUT_CTRL_OFFS                                                              (0x00000024)
#define HWIO_GENI_OUTPUT_CTRL_RMSK                                                                    0x7f
#define HWIO_GENI_OUTPUT_CTRL_IN(x)      \
        in_dword_masked(HWIO_GENI_OUTPUT_CTRL_ADDR(x), HWIO_GENI_OUTPUT_CTRL_RMSK)
#define HWIO_GENI_OUTPUT_CTRL_INM(x, m)      \
        in_dword_masked(HWIO_GENI_OUTPUT_CTRL_ADDR(x), m)
#define HWIO_GENI_OUTPUT_CTRL_OUT(x, v)      \
        out_dword(HWIO_GENI_OUTPUT_CTRL_ADDR(x),v)
#define HWIO_GENI_OUTPUT_CTRL_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_GENI_OUTPUT_CTRL_ADDR(x),m,v,HWIO_GENI_OUTPUT_CTRL_IN(x))
#define HWIO_GENI_OUTPUT_CTRL_IO_OUTPUT_CTRL_BMSK                                                     0x7f
#define HWIO_GENI_OUTPUT_CTRL_IO_OUTPUT_CTRL_SHFT                                                      0x0

#define HWIO_GENI_CGC_CTRL_ADDR(x)                                                              ((x) + 0x00000028)
#define HWIO_GENI_CGC_CTRL_OFFS                                                                 (0x00000028)
#define HWIO_GENI_CGC_CTRL_RMSK                                                                      0x37f
#define HWIO_GENI_CGC_CTRL_IN(x)      \
        in_dword_masked(HWIO_GENI_CGC_CTRL_ADDR(x), HWIO_GENI_CGC_CTRL_RMSK)
#define HWIO_GENI_CGC_CTRL_INM(x, m)      \
        in_dword_masked(HWIO_GENI_CGC_CTRL_ADDR(x), m)
#define HWIO_GENI_CGC_CTRL_OUT(x, v)      \
        out_dword(HWIO_GENI_CGC_CTRL_ADDR(x),v)
#define HWIO_GENI_CGC_CTRL_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_GENI_CGC_CTRL_ADDR(x),m,v,HWIO_GENI_CGC_CTRL_IN(x))
#define HWIO_GENI_CGC_CTRL_PROG_RAM_SCLK_OFF_BMSK                                                    0x200
#define HWIO_GENI_CGC_CTRL_PROG_RAM_SCLK_OFF_SHFT                                                      0x9
#define HWIO_GENI_CGC_CTRL_PROG_RAM_HCLK_OFF_BMSK                                                    0x100
#define HWIO_GENI_CGC_CTRL_PROG_RAM_HCLK_OFF_SHFT                                                      0x8
#define HWIO_GENI_CGC_CTRL_EXT_CLK_CGC_ON_BMSK                                                        0x40
#define HWIO_GENI_CGC_CTRL_EXT_CLK_CGC_ON_SHFT                                                         0x6
#define HWIO_GENI_CGC_CTRL_RX_CLK_CGC_ON_BMSK                                                         0x20
#define HWIO_GENI_CGC_CTRL_RX_CLK_CGC_ON_SHFT                                                          0x5
#define HWIO_GENI_CGC_CTRL_TX_CLK_CGC_ON_BMSK                                                         0x10
#define HWIO_GENI_CGC_CTRL_TX_CLK_CGC_ON_SHFT                                                          0x4
#define HWIO_GENI_CGC_CTRL_SCLK_CGC_ON_BMSK                                                            0x8
#define HWIO_GENI_CGC_CTRL_SCLK_CGC_ON_SHFT                                                            0x3
#define HWIO_GENI_CGC_CTRL_DATA_AHB_CLK_CGC_ON_BMSK                                                    0x4
#define HWIO_GENI_CGC_CTRL_DATA_AHB_CLK_CGC_ON_SHFT                                                    0x2
#define HWIO_GENI_CGC_CTRL_CFG_AHB_WR_CLK_CGC_ON_BMSK                                                  0x2
#define HWIO_GENI_CGC_CTRL_CFG_AHB_WR_CLK_CGC_ON_SHFT                                                  0x1
#define HWIO_GENI_CGC_CTRL_CFG_AHB_CLK_CGC_ON_BMSK                                                     0x1
#define HWIO_GENI_CGC_CTRL_CFG_AHB_CLK_CGC_ON_SHFT                                                     0x0


#define HWIO_GENI_STATUS_ADDR(x)                                                                ((x) + 0x00000040)
#define HWIO_GENI_STATUS_OFFS                                                                   (0x00000040)
#define HWIO_GENI_STATUS_RMSK                                                                     0x1fffff
#define HWIO_GENI_STATUS_IN(x)      \
        in_dword_masked(HWIO_GENI_STATUS_ADDR(x), HWIO_GENI_STATUS_RMSK)
#define HWIO_GENI_STATUS_INM(x, m)      \
        in_dword_masked(HWIO_GENI_STATUS_ADDR(x), m)
#define HWIO_GENI_STATUS_S_GENI_CMD_FSM_STATE_BMSK                                                0x1f0000
#define HWIO_GENI_STATUS_S_GENI_CMD_FSM_STATE_SHFT                                                    0x10
#define HWIO_GENI_STATUS_NOT_USED_BITS_3_BMSK                                                       0xe000
#define HWIO_GENI_STATUS_NOT_USED_BITS_3_SHFT                                                          0xd
#define HWIO_GENI_STATUS_S_GENI_CMD_ACTIVE_BMSK                                                     0x1000
#define HWIO_GENI_STATUS_S_GENI_CMD_ACTIVE_SHFT                                                        0xc
#define HWIO_GENI_STATUS_NOT_USED_BITS_2_BMSK                                                        0xe00
#define HWIO_GENI_STATUS_NOT_USED_BITS_2_SHFT                                                          0x9
#define HWIO_GENI_STATUS_M_GENI_CMD_FSM_STATE_BMSK                                                   0x1f0
#define HWIO_GENI_STATUS_M_GENI_CMD_FSM_STATE_SHFT                                                     0x4
#define HWIO_GENI_STATUS_NOT_USED_BITS_1_BMSK                                                          0xe
#define HWIO_GENI_STATUS_NOT_USED_BITS_1_SHFT                                                          0x1
#define HWIO_GENI_STATUS_M_GENI_CMD_ACTIVE_BMSK                                                        0x1
#define HWIO_GENI_STATUS_M_GENI_CMD_ACTIVE_SHFT                                                        0x0


#define HWIO_GENI_SER_M_CLK_CFG_ADDR(x)                                                         ((x) + 0x00000048)
#define HWIO_GENI_SER_M_CLK_CFG_OFFS                                                            (0x00000048)
#define HWIO_GENI_SER_M_CLK_CFG_RMSK                                                                0xfff1
#define HWIO_GENI_SER_M_CLK_CFG_IN(x)      \
        in_dword_masked(HWIO_GENI_SER_M_CLK_CFG_ADDR(x), HWIO_GENI_SER_M_CLK_CFG_RMSK)
#define HWIO_GENI_SER_M_CLK_CFG_INM(x, m)      \
        in_dword_masked(HWIO_GENI_SER_M_CLK_CFG_ADDR(x), m)
#define HWIO_GENI_SER_M_CLK_CFG_OUT(x, v)      \
        out_dword(HWIO_GENI_SER_M_CLK_CFG_ADDR(x),v)
#define HWIO_GENI_SER_M_CLK_CFG_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_GENI_SER_M_CLK_CFG_ADDR(x),m,v,HWIO_GENI_SER_M_CLK_CFG_IN(x))
#define HWIO_GENI_SER_M_CLK_CFG_CLK_DIV_VALUE_BMSK                                                  0xfff0
#define HWIO_GENI_SER_M_CLK_CFG_CLK_DIV_VALUE_SHFT                                                     0x4
#define HWIO_GENI_SER_M_CLK_CFG_SER_CLK_EN_BMSK                                                        0x1
#define HWIO_GENI_SER_M_CLK_CFG_SER_CLK_EN_SHFT                                                        0x0

#define HWIO_GENI_SER_S_CLK_CFG_ADDR(x)                                                         ((x) + 0x0000004c)
#define HWIO_GENI_SER_S_CLK_CFG_OFFS                                                            (0x0000004c)
#define HWIO_GENI_SER_S_CLK_CFG_RMSK                                                                0xfff1
#define HWIO_GENI_SER_S_CLK_CFG_IN(x)      \
        in_dword_masked(HWIO_GENI_SER_S_CLK_CFG_ADDR(x), HWIO_GENI_SER_S_CLK_CFG_RMSK)
#define HWIO_GENI_SER_S_CLK_CFG_INM(x, m)      \
        in_dword_masked(HWIO_GENI_SER_S_CLK_CFG_ADDR(x), m)
#define HWIO_GENI_SER_S_CLK_CFG_OUT(x, v)      \
        out_dword(HWIO_GENI_SER_S_CLK_CFG_ADDR(x),v)
#define HWIO_GENI_SER_S_CLK_CFG_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_GENI_SER_S_CLK_CFG_ADDR(x),m,v,HWIO_GENI_SER_S_CLK_CFG_IN(x))
#define HWIO_GENI_SER_S_CLK_CFG_CLK_DIV_VALUE_BMSK                                                  0xfff0
#define HWIO_GENI_SER_S_CLK_CFG_CLK_DIV_VALUE_SHFT                                                     0x4
#define HWIO_GENI_SER_S_CLK_CFG_SER_CLK_EN_BMSK                                                        0x1
#define HWIO_GENI_SER_S_CLK_CFG_SER_CLK_EN_SHFT                                                        0x0

#define HWIO_GENI_DFS_IF_CFG_ADDR(x)                                                            ((x) + 0x00000080)
#define HWIO_GENI_DFS_IF_CFG_OFFS                                                               (0x00000080)
#define HWIO_GENI_DFS_IF_CFG_RMSK                                                                    0x701
#define HWIO_GENI_DFS_IF_CFG_IN(x)      \
        in_dword_masked(HWIO_GENI_DFS_IF_CFG_ADDR(x), HWIO_GENI_DFS_IF_CFG_RMSK)
#define HWIO_GENI_DFS_IF_CFG_INM(x, m)      \
        in_dword_masked(HWIO_GENI_DFS_IF_CFG_ADDR(x), m)
#define HWIO_GENI_DFS_IF_CFG_OUT(x, v)      \
        out_dword(HWIO_GENI_DFS_IF_CFG_ADDR(x),v)
#define HWIO_GENI_DFS_IF_CFG_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_GENI_DFS_IF_CFG_ADDR(x),m,v,HWIO_GENI_DFS_IF_CFG_IN(x))
#define HWIO_GENI_DFS_IF_CFG_NUM_WAIT_STATES_BMSK                                                    0x700
#define HWIO_GENI_DFS_IF_CFG_NUM_WAIT_STATES_SHFT                                                      0x8
#define HWIO_GENI_DFS_IF_CFG_DFS_IF_EN_BMSK                                                            0x1
#define HWIO_GENI_DFS_IF_CFG_DFS_IF_EN_SHFT                                                            0x0

#define HWIO_GENI_FORCE_DEFAULT_REG_ADDR(x)                                                     ((x) + 0x00000020)
#define HWIO_GENI_FORCE_DEFAULT_REG_OFFS                                                        (0x00000020)
#define HWIO_GENI_FORCE_DEFAULT_REG_RMSK                                                               0x1
#define HWIO_GENI_FORCE_DEFAULT_REG_OUT(x, v)      \
        out_dword(HWIO_GENI_FORCE_DEFAULT_REG_ADDR(x),v)
#define HWIO_GENI_FORCE_DEFAULT_REG_FORCE_DEFAULT_BMSK                                                 0x1
#define HWIO_GENI_FORCE_DEFAULT_REG_FORCE_DEFAULT_SHFT                                                 0x0

//
// MODULE: GENI_DATA
//

#define HWIO_GENI_M_CMD0_ADDR(x)                                                          ((x) + 0x00000000)
#define HWIO_GENI_M_CMD0_OFFS                                                             (0x00000000)
#define HWIO_GENI_M_CMD0_RMSK                                                             0xf9ffffff
#define HWIO_GENI_M_CMD0_IN(x)      \
        in_dword_masked(HWIO_GENI_M_CMD0_ADDR(x), HWIO_GENI_M_CMD0_RMSK)
#define HWIO_GENI_M_CMD0_INM(x, m)      \
        in_dword_masked(HWIO_GENI_M_CMD0_ADDR(x), m)
#define HWIO_GENI_M_CMD0_OUT(x, v)      \
        out_dword(HWIO_GENI_M_CMD0_ADDR(x),v)
#define HWIO_GENI_M_CMD0_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_GENI_M_CMD0_ADDR(x),m,v,HWIO_GENI_M_CMD0_IN(x))
#define HWIO_GENI_M_CMD0_OPCODE_BMSK                                                      0xf8000000
#define HWIO_GENI_M_CMD0_OPCODE_SHFT                                                            0x1b
#define HWIO_GENI_M_CMD0_GNRL_PURP_BMSK                                                    0x1000000
#define HWIO_GENI_M_CMD0_GNRL_PURP_SHFT                                                         0x18
#define HWIO_GENI_M_CMD0_PARAM_BMSK                                                         0xffffff
#define HWIO_GENI_M_CMD0_PARAM_SHFT                                                              0x0

#define HWIO_GENI_M_IRQ_STATUS_ADDR(x)                                                    ((x) + 0x00000010)
#define HWIO_GENI_M_IRQ_STATUS_OFFS                                                       (0x00000010)
#define HWIO_GENI_M_IRQ_STATUS_RMSK                                                       0xffc07fff
#define HWIO_GENI_M_IRQ_STATUS_IN(x)      \
        in_dword_masked(HWIO_GENI_M_IRQ_STATUS_ADDR(x), HWIO_GENI_M_IRQ_STATUS_RMSK)
#define HWIO_GENI_M_IRQ_STATUS_INM(x, m)      \
        in_dword_masked(HWIO_GENI_M_IRQ_STATUS_ADDR(x), m)
#define HWIO_GENI_M_IRQ_STATUS_SEC_IRQ_BMSK                                               0x80000000
#define HWIO_GENI_M_IRQ_STATUS_SEC_IRQ_SHFT                                                     0x1f
#define HWIO_GENI_M_IRQ_STATUS_TX_FIFO_WATERMARK_BMSK                                     0x40000000
#define HWIO_GENI_M_IRQ_STATUS_TX_FIFO_WATERMARK_SHFT                                           0x1e
#define HWIO_GENI_M_IRQ_STATUS_TX_FIFO_WR_ERR_BMSK                                        0x20000000
#define HWIO_GENI_M_IRQ_STATUS_TX_FIFO_WR_ERR_SHFT                                              0x1d
#define HWIO_GENI_M_IRQ_STATUS_TX_FIFO_RD_ERR_BMSK                                        0x10000000
#define HWIO_GENI_M_IRQ_STATUS_TX_FIFO_RD_ERR_SHFT                                              0x1c
#define HWIO_GENI_M_IRQ_STATUS_RX_FIFO_LAST_BMSK                                           0x8000000
#define HWIO_GENI_M_IRQ_STATUS_RX_FIFO_LAST_SHFT                                                0x1b
#define HWIO_GENI_M_IRQ_STATUS_RX_FIFO_WATERMARK_BMSK                                      0x4000000
#define HWIO_GENI_M_IRQ_STATUS_RX_FIFO_WATERMARK_SHFT                                           0x1a
#define HWIO_GENI_M_IRQ_STATUS_RX_FIFO_WR_ERR_BMSK                                         0x2000000
#define HWIO_GENI_M_IRQ_STATUS_RX_FIFO_WR_ERR_SHFT                                              0x19
#define HWIO_GENI_M_IRQ_STATUS_RX_FIFO_RD_ERR_BMSK                                         0x1000000
#define HWIO_GENI_M_IRQ_STATUS_RX_FIFO_RD_ERR_SHFT                                              0x18
#define HWIO_GENI_M_IRQ_STATUS_IO_DATA_ASSERT_BMSK                                          0x800000
#define HWIO_GENI_M_IRQ_STATUS_IO_DATA_ASSERT_SHFT                                              0x17
#define HWIO_GENI_M_IRQ_STATUS_IO_DATA_DEASSERT_BMSK                                        0x400000
#define HWIO_GENI_M_IRQ_STATUS_IO_DATA_DEASSERT_SHFT                                            0x16
#define HWIO_GENI_M_IRQ_STATUS_M_GP_IRQ_5_BMSK                                                0x4000
#define HWIO_GENI_M_IRQ_STATUS_M_GP_IRQ_5_SHFT                                                   0xe
#define HWIO_GENI_M_IRQ_STATUS_M_GP_IRQ_4_BMSK                                                0x2000
#define HWIO_GENI_M_IRQ_STATUS_M_GP_IRQ_4_SHFT                                                   0xd
#define HWIO_GENI_M_IRQ_STATUS_M_GP_IRQ_3_BMSK                                                0x1000
#define HWIO_GENI_M_IRQ_STATUS_M_GP_IRQ_3_SHFT                                                   0xc
#define HWIO_GENI_M_IRQ_STATUS_M_GP_IRQ_2_BMSK                                                 0x800
#define HWIO_GENI_M_IRQ_STATUS_M_GP_IRQ_2_SHFT                                                   0xb
#define HWIO_GENI_M_IRQ_STATUS_M_GP_IRQ_1_BMSK                                                 0x400
#define HWIO_GENI_M_IRQ_STATUS_M_GP_IRQ_1_SHFT                                                   0xa
#define HWIO_GENI_M_IRQ_STATUS_M_GP_IRQ_0_BMSK                                                 0x200
#define HWIO_GENI_M_IRQ_STATUS_M_GP_IRQ_0_SHFT                                                   0x9
#define HWIO_GENI_M_IRQ_STATUS_M_GP_SYNC_IRQ_0_BMSK                                            0x100
#define HWIO_GENI_M_IRQ_STATUS_M_GP_SYNC_IRQ_0_SHFT                                              0x8
#define HWIO_GENI_M_IRQ_STATUS_M_RX_IRQ_BMSK                                                    0x80
#define HWIO_GENI_M_IRQ_STATUS_M_RX_IRQ_SHFT                                                     0x7
#define HWIO_GENI_M_IRQ_STATUS_M_TIMESTAMP_BMSK                                                 0x40
#define HWIO_GENI_M_IRQ_STATUS_M_TIMESTAMP_SHFT                                                  0x6
#define HWIO_GENI_M_IRQ_STATUS_M_CMD_ABORT_BMSK                                                 0x20
#define HWIO_GENI_M_IRQ_STATUS_M_CMD_ABORT_SHFT                                                  0x5
#define HWIO_GENI_M_IRQ_STATUS_M_CMD_CANCEL_BMSK                                                0x10
#define HWIO_GENI_M_IRQ_STATUS_M_CMD_CANCEL_SHFT                                                 0x4
#define HWIO_GENI_M_IRQ_STATUS_M_CMD_FAILURE_BMSK                                                0x8
#define HWIO_GENI_M_IRQ_STATUS_M_CMD_FAILURE_SHFT                                                0x3
#define HWIO_GENI_M_IRQ_STATUS_M_ILLEGAL_CMD_BMSK                                                0x4
#define HWIO_GENI_M_IRQ_STATUS_M_ILLEGAL_CMD_SHFT                                                0x2
#define HWIO_GENI_M_IRQ_STATUS_M_CMD_OVERRUN_BMSK                                                0x2
#define HWIO_GENI_M_IRQ_STATUS_M_CMD_OVERRUN_SHFT                                                0x1
#define HWIO_GENI_M_IRQ_STATUS_M_CMD_DONE_BMSK                                                   0x1
#define HWIO_GENI_M_IRQ_STATUS_M_CMD_DONE_SHFT                                                   0x0

#define HWIO_GENI_M_IRQ_ENABLE_ADDR(x)                                                    ((x) + 0x00000014)
#define HWIO_GENI_M_IRQ_ENABLE_OFFS                                                       (0x00000014)
#define HWIO_GENI_M_IRQ_ENABLE_RMSK                                                       0xffc07fff
#define HWIO_GENI_M_IRQ_ENABLE_IN(x)      \
        in_dword_masked(HWIO_GENI_M_IRQ_ENABLE_ADDR(x), HWIO_GENI_M_IRQ_ENABLE_RMSK)
#define HWIO_GENI_M_IRQ_ENABLE_INM(x, m)      \
        in_dword_masked(HWIO_GENI_M_IRQ_ENABLE_ADDR(x), m)
#define HWIO_GENI_M_IRQ_ENABLE_OUT(x, v)      \
        out_dword(HWIO_GENI_M_IRQ_ENABLE_ADDR(x),v)
#define HWIO_GENI_M_IRQ_ENABLE_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_GENI_M_IRQ_ENABLE_ADDR(x),m,v,HWIO_GENI_M_IRQ_ENABLE_IN(x))
#define HWIO_GENI_M_IRQ_ENABLE_SEC_IRQ_EN_BMSK                                            0x80000000
#define HWIO_GENI_M_IRQ_ENABLE_SEC_IRQ_EN_SHFT                                                  0x1f
#define HWIO_GENI_M_IRQ_ENABLE_TX_FIFO_WATERMARK_EN_BMSK                                  0x40000000
#define HWIO_GENI_M_IRQ_ENABLE_TX_FIFO_WATERMARK_EN_SHFT                                        0x1e
#define HWIO_GENI_M_IRQ_ENABLE_TX_FIFO_WR_ERR_EN_BMSK                                     0x20000000
#define HWIO_GENI_M_IRQ_ENABLE_TX_FIFO_WR_ERR_EN_SHFT                                           0x1d
#define HWIO_GENI_M_IRQ_ENABLE_TX_FIFO_RD_ERR_EN_BMSK                                     0x10000000
#define HWIO_GENI_M_IRQ_ENABLE_TX_FIFO_RD_ERR_EN_SHFT                                           0x1c
#define HWIO_GENI_M_IRQ_ENABLE_RX_FIFO_LAST_EN_BMSK                                        0x8000000
#define HWIO_GENI_M_IRQ_ENABLE_RX_FIFO_LAST_EN_SHFT                                             0x1b
#define HWIO_GENI_M_IRQ_ENABLE_RX_FIFO_WATERMARK_EN_BMSK                                   0x4000000
#define HWIO_GENI_M_IRQ_ENABLE_RX_FIFO_WATERMARK_EN_SHFT                                        0x1a
#define HWIO_GENI_M_IRQ_ENABLE_RX_FIFO_WR_ERR_EN_BMSK                                      0x2000000
#define HWIO_GENI_M_IRQ_ENABLE_RX_FIFO_WR_ERR_EN_SHFT                                           0x19
#define HWIO_GENI_M_IRQ_ENABLE_RX_FIFO_RD_ERR_EN_BMSK                                      0x1000000
#define HWIO_GENI_M_IRQ_ENABLE_RX_FIFO_RD_ERR_EN_SHFT                                           0x18
#define HWIO_GENI_M_IRQ_ENABLE_IO_DATA_ASSERT_EN_BMSK                                       0x800000
#define HWIO_GENI_M_IRQ_ENABLE_IO_DATA_ASSERT_EN_SHFT                                           0x17
#define HWIO_GENI_M_IRQ_ENABLE_IO_DATA_DEASSERT_EN_BMSK                                     0x400000
#define HWIO_GENI_M_IRQ_ENABLE_IO_DATA_DEASSERT_EN_SHFT                                         0x16
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_IRQ_5_EN_BMSK                                             0x4000
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_IRQ_5_EN_SHFT                                                0xe
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_IRQ_4_EN_BMSK                                             0x2000
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_IRQ_4_EN_SHFT                                                0xd
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_IRQ_3_EN_BMSK                                             0x1000
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_IRQ_3_EN_SHFT                                                0xc
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_IRQ_2_EN_BMSK                                              0x800
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_IRQ_2_EN_SHFT                                                0xb
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_IRQ_1_EN_BMSK                                              0x400
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_IRQ_1_EN_SHFT                                                0xa
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_IRQ_0_EN_BMSK                                              0x200
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_IRQ_0_EN_SHFT                                                0x9
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_SYNC_IRQ_0_EN_BMSK                                         0x100
#define HWIO_GENI_M_IRQ_ENABLE_M_GP_SYNC_IRQ_0_EN_SHFT                                           0x8
#define HWIO_GENI_M_IRQ_ENABLE_M_RX_IRQ_EN_BMSK                                                 0x80
#define HWIO_GENI_M_IRQ_ENABLE_M_RX_IRQ_EN_SHFT                                                  0x7
#define HWIO_GENI_M_IRQ_ENABLE_M_TIMESTAMP_EN_BMSK                                              0x40
#define HWIO_GENI_M_IRQ_ENABLE_M_TIMESTAMP_EN_SHFT                                               0x6
#define HWIO_GENI_M_IRQ_ENABLE_M_CMD_ABORT_EN_BMSK                                              0x20
#define HWIO_GENI_M_IRQ_ENABLE_M_CMD_ABORT_EN_SHFT                                               0x5
#define HWIO_GENI_M_IRQ_ENABLE_M_CMD_CANCEL_EN_BMSK                                             0x10
#define HWIO_GENI_M_IRQ_ENABLE_M_CMD_CANCEL_EN_SHFT                                              0x4
#define HWIO_GENI_M_IRQ_ENABLE_M_CMD_FAILURE_EN_BMSK                                             0x8
#define HWIO_GENI_M_IRQ_ENABLE_M_CMD_FAILURE_EN_SHFT                                             0x3
#define HWIO_GENI_M_IRQ_ENABLE_M_ILLEGAL_CMD_EN_BMSK                                             0x4
#define HWIO_GENI_M_IRQ_ENABLE_M_ILLEGAL_CMD_EN_SHFT                                             0x2
#define HWIO_GENI_M_IRQ_ENABLE_M_CMD_OVERRUN_EN_BMSK                                             0x2
#define HWIO_GENI_M_IRQ_ENABLE_M_CMD_OVERRUN_EN_SHFT                                             0x1
#define HWIO_GENI_M_IRQ_ENABLE_M_CMD_DONE_EN_BMSK                                                0x1
#define HWIO_GENI_M_IRQ_ENABLE_M_CMD_DONE_EN_SHFT                                                0x0

#define HWIO_GENI_M_IRQ_CLEAR_ADDR(x)                                                     ((x) + 0x00000018)
#define HWIO_GENI_M_IRQ_CLEAR_OFFS                                                        (0x00000018)
#define HWIO_GENI_M_IRQ_CLEAR_RMSK                                                        0xffc07fff
#define HWIO_GENI_M_IRQ_CLEAR_OUT(x, v)      \
        out_dword(HWIO_GENI_M_IRQ_CLEAR_ADDR(x),v)
#define HWIO_GENI_M_IRQ_CLEAR_SEC_IRQ_CLEAR_BMSK                                          0x80000000
#define HWIO_GENI_M_IRQ_CLEAR_SEC_IRQ_CLEAR_SHFT                                                0x1f
#define HWIO_GENI_M_IRQ_CLEAR_TX_FIFO_WATERMARK_CLEAR_BMSK                                0x40000000
#define HWIO_GENI_M_IRQ_CLEAR_TX_FIFO_WATERMARK_CLEAR_SHFT                                      0x1e
#define HWIO_GENI_M_IRQ_CLEAR_TX_FIFO_WR_ERR_CLEAR_BMSK                                   0x20000000
#define HWIO_GENI_M_IRQ_CLEAR_TX_FIFO_WR_ERR_CLEAR_SHFT                                         0x1d
#define HWIO_GENI_M_IRQ_CLEAR_TX_FIFO_RD_ERR_CLEAR_BMSK                                   0x10000000
#define HWIO_GENI_M_IRQ_CLEAR_TX_FIFO_RD_ERR_CLEAR_SHFT                                         0x1c
#define HWIO_GENI_M_IRQ_CLEAR_RX_FIFO_LAST_CLEAR_BMSK                                      0x8000000
#define HWIO_GENI_M_IRQ_CLEAR_RX_FIFO_LAST_CLEAR_SHFT                                           0x1b
#define HWIO_GENI_M_IRQ_CLEAR_RX_FIFO_WATERMARK_CLEAR_BMSK                                 0x4000000
#define HWIO_GENI_M_IRQ_CLEAR_RX_FIFO_WATERMARK_CLEAR_SHFT                                      0x1a
#define HWIO_GENI_M_IRQ_CLEAR_RX_FIFO_WR_ERR_CLEAR_BMSK                                    0x2000000
#define HWIO_GENI_M_IRQ_CLEAR_RX_FIFO_WR_ERR_CLEAR_SHFT                                         0x19
#define HWIO_GENI_M_IRQ_CLEAR_RX_FIFO_RD_ERR_CLEAR_BMSK                                    0x1000000
#define HWIO_GENI_M_IRQ_CLEAR_RX_FIFO_RD_ERR_CLEAR_SHFT                                         0x18
#define HWIO_GENI_M_IRQ_CLEAR_IO_DATA_ASSERT_CLEAR_BMSK                                     0x800000
#define HWIO_GENI_M_IRQ_CLEAR_IO_DATA_ASSERT_CLEAR_SHFT                                         0x17
#define HWIO_GENI_M_IRQ_CLEAR_IO_DATA_DEASSERT_CLEAR_BMSK                                   0x400000
#define HWIO_GENI_M_IRQ_CLEAR_IO_DATA_DEASSERT_CLEAR_SHFT                                       0x16
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_IRQ_5_CLEAR_BMSK                                           0x4000
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_IRQ_5_CLEAR_SHFT                                              0xe
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_IRQ_4_CLEAR_BMSK                                           0x2000
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_IRQ_4_CLEAR_SHFT                                              0xd
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_IRQ_3_CLEAR_BMSK                                           0x1000
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_IRQ_3_CLEAR_SHFT                                              0xc
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_IRQ_2_CLEAR_BMSK                                            0x800
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_IRQ_2_CLEAR_SHFT                                              0xb
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_IRQ_1_CLEAR_BMSK                                            0x400
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_IRQ_1_CLEAR_SHFT                                              0xa
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_IRQ_0_CLEAR_BMSK                                            0x200
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_IRQ_0_CLEAR_SHFT                                              0x9
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_SYNC_IRQ_0_CLEAR_BMSK                                       0x100
#define HWIO_GENI_M_IRQ_CLEAR_M_GP_SYNC_IRQ_0_CLEAR_SHFT                                         0x8
#define HWIO_GENI_M_IRQ_CLEAR_M_RX_IRQ_CLEAR_BMSK                                               0x80
#define HWIO_GENI_M_IRQ_CLEAR_M_RX_IRQ_CLEAR_SHFT                                                0x7
#define HWIO_GENI_M_IRQ_CLEAR_M_TIMESTAMP_CLEAR_BMSK                                            0x40
#define HWIO_GENI_M_IRQ_CLEAR_M_TIMESTAMP_CLEAR_SHFT                                             0x6
#define HWIO_GENI_M_IRQ_CLEAR_M_CMD_ABORT_CLEAR_BMSK                                            0x20
#define HWIO_GENI_M_IRQ_CLEAR_M_CMD_ABORT_CLEAR_SHFT                                             0x5
#define HWIO_GENI_M_IRQ_CLEAR_M_CMD_CANCEL_CLEAR_BMSK                                           0x10
#define HWIO_GENI_M_IRQ_CLEAR_M_CMD_CANCEL_CLEAR_SHFT                                            0x4
#define HWIO_GENI_M_IRQ_CLEAR_M_CMD_FAILURE_CLEAR_BMSK                                           0x8
#define HWIO_GENI_M_IRQ_CLEAR_M_CMD_FAILURE_CLEAR_SHFT                                           0x3
#define HWIO_GENI_M_IRQ_CLEAR_M_ILLEGAL_CMD_CLEAR_BMSK                                           0x4
#define HWIO_GENI_M_IRQ_CLEAR_M_ILLEGAL_CMD_CLEAR_SHFT                                           0x2
#define HWIO_GENI_M_IRQ_CLEAR_M_CMD_OVERRUN_CLEAR_BMSK                                           0x2
#define HWIO_GENI_M_IRQ_CLEAR_M_CMD_OVERRUN_CLEAR_SHFT                                           0x1
#define HWIO_GENI_M_IRQ_CLEAR_M_CMD_DONE_CLEAR_BMSK                                              0x1
#define HWIO_GENI_M_IRQ_CLEAR_M_CMD_DONE_CLEAR_SHFT                                              0x0

#define HWIO_GENI_S_CMD0_ADDR(x)                                                          ((x) + 0x00000030)
#define HWIO_GENI_S_CMD0_OFFS                                                             (0x00000030)
#define HWIO_GENI_S_CMD0_RMSK                                                             0xf9ffffff
#define HWIO_GENI_S_CMD0_IN(x)      \
        in_dword_masked(HWIO_GENI_S_CMD0_ADDR(x), HWIO_GENI_S_CMD0_RMSK)
#define HWIO_GENI_S_CMD0_INM(x, m)      \
        in_dword_masked(HWIO_GENI_S_CMD0_ADDR(x), m)
#define HWIO_GENI_S_CMD0_OUT(x, v)      \
        out_dword(HWIO_GENI_S_CMD0_ADDR(x),v)
#define HWIO_GENI_S_CMD0_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_GENI_S_CMD0_ADDR(x),m,v,HWIO_GENI_S_CMD0_IN(x))
#define HWIO_GENI_S_CMD0_OPCODE_BMSK                                                      0xf8000000
#define HWIO_GENI_S_CMD0_OPCODE_SHFT                                                            0x1b
#define HWIO_GENI_S_CMD0_GNRL_PURP_BMSK                                                    0x1000000
#define HWIO_GENI_S_CMD0_GNRL_PURP_SHFT                                                         0x18
#define HWIO_GENI_S_CMD0_PARAM_BMSK                                                         0xffffff
#define HWIO_GENI_S_CMD0_PARAM_SHFT                                                              0x0

#define HWIO_GENI_S_IRQ_STATUS_ADDR(x)                                                    ((x) + 0x00000040)
#define HWIO_GENI_S_IRQ_STATUS_OFFS                                                       (0x00000040)
#define HWIO_GENI_S_IRQ_STATUS_RMSK                                                        0xfc07f3f
#define HWIO_GENI_S_IRQ_STATUS_IN(x)      \
        in_dword_masked(HWIO_GENI_S_IRQ_STATUS_ADDR(x), HWIO_GENI_S_IRQ_STATUS_RMSK)
#define HWIO_GENI_S_IRQ_STATUS_INM(x, m)      \
        in_dword_masked(HWIO_GENI_S_IRQ_STATUS_ADDR(x), m)
#define HWIO_GENI_S_IRQ_STATUS_RX_FIFO_LAST_BMSK                                           0x8000000
#define HWIO_GENI_S_IRQ_STATUS_RX_FIFO_LAST_SHFT                                                0x1b
#define HWIO_GENI_S_IRQ_STATUS_RX_FIFO_WATERMARK_BMSK                                      0x4000000
#define HWIO_GENI_S_IRQ_STATUS_RX_FIFO_WATERMARK_SHFT                                           0x1a
#define HWIO_GENI_S_IRQ_STATUS_RX_FIFO_WR_ERR_BMSK                                         0x2000000
#define HWIO_GENI_S_IRQ_STATUS_RX_FIFO_WR_ERR_SHFT                                              0x19
#define HWIO_GENI_S_IRQ_STATUS_RX_FIFO_RD_ERR_BMSK                                         0x1000000
#define HWIO_GENI_S_IRQ_STATUS_RX_FIFO_RD_ERR_SHFT                                              0x18
#define HWIO_GENI_S_IRQ_STATUS_IO_DATA_ASSERT_BMSK                                          0x800000
#define HWIO_GENI_S_IRQ_STATUS_IO_DATA_ASSERT_SHFT                                              0x17
#define HWIO_GENI_S_IRQ_STATUS_IO_DATA_DEASSERT_BMSK                                        0x400000
#define HWIO_GENI_S_IRQ_STATUS_IO_DATA_DEASSERT_SHFT                                            0x16
#define HWIO_GENI_S_IRQ_STATUS_S_GP_IRQ_5_BMSK                                                0x4000
#define HWIO_GENI_S_IRQ_STATUS_S_GP_IRQ_5_SHFT                                                   0xe
#define HWIO_GENI_S_IRQ_STATUS_S_GP_IRQ_4_BMSK                                                0x2000
#define HWIO_GENI_S_IRQ_STATUS_S_GP_IRQ_4_SHFT                                                   0xd
#define HWIO_GENI_S_IRQ_STATUS_S_GP_IRQ_3_BMSK                                                0x1000
#define HWIO_GENI_S_IRQ_STATUS_S_GP_IRQ_3_SHFT                                                   0xc
#define HWIO_GENI_S_IRQ_STATUS_S_GP_IRQ_2_BMSK                                                 0x800
#define HWIO_GENI_S_IRQ_STATUS_S_GP_IRQ_2_SHFT                                                   0xb
#define HWIO_GENI_S_IRQ_STATUS_S_GP_IRQ_1_BMSK                                                 0x400
#define HWIO_GENI_S_IRQ_STATUS_S_GP_IRQ_1_SHFT                                                   0xa
#define HWIO_GENI_S_IRQ_STATUS_S_GP_IRQ_0_BMSK                                                 0x200
#define HWIO_GENI_S_IRQ_STATUS_S_GP_IRQ_0_SHFT                                                   0x9
#define HWIO_GENI_S_IRQ_STATUS_S_GP_SYNC_IRQ_0_BMSK                                            0x100
#define HWIO_GENI_S_IRQ_STATUS_S_GP_SYNC_IRQ_0_SHFT                                              0x8
#define HWIO_GENI_S_IRQ_STATUS_S_CMD_ABORT_BMSK                                                 0x20
#define HWIO_GENI_S_IRQ_STATUS_S_CMD_ABORT_SHFT                                                  0x5
#define HWIO_GENI_S_IRQ_STATUS_S_CMD_CANCEL_BMSK                                                0x10
#define HWIO_GENI_S_IRQ_STATUS_S_CMD_CANCEL_SHFT                                                 0x4
#define HWIO_GENI_S_IRQ_STATUS_S_CMD_FAILURE_BMSK                                                0x8
#define HWIO_GENI_S_IRQ_STATUS_S_CMD_FAILURE_SHFT                                                0x3
#define HWIO_GENI_S_IRQ_STATUS_S_ILLEGAL_CMD_BMSK                                                0x4
#define HWIO_GENI_S_IRQ_STATUS_S_ILLEGAL_CMD_SHFT                                                0x2
#define HWIO_GENI_S_IRQ_STATUS_S_CMD_OVERRUN_BMSK                                                0x2
#define HWIO_GENI_S_IRQ_STATUS_S_CMD_OVERRUN_SHFT                                                0x1
#define HWIO_GENI_S_IRQ_STATUS_S_CMD_DONE_BMSK                                                   0x1
#define HWIO_GENI_S_IRQ_STATUS_S_CMD_DONE_SHFT                                                   0x0

#define HWIO_GENI_S_IRQ_ENABLE_ADDR(x)                                                    ((x) + 0x00000044)
#define HWIO_GENI_S_IRQ_ENABLE_OFFS                                                       (0x00000044)
#define HWIO_GENI_S_IRQ_ENABLE_RMSK                                                        0xfc07f3f
#define HWIO_GENI_S_IRQ_ENABLE_IN(x)      \
        in_dword_masked(HWIO_GENI_S_IRQ_ENABLE_ADDR(x), HWIO_GENI_S_IRQ_ENABLE_RMSK)
#define HWIO_GENI_S_IRQ_ENABLE_INM(x, m)      \
        in_dword_masked(HWIO_GENI_S_IRQ_ENABLE_ADDR(x), m)
#define HWIO_GENI_S_IRQ_ENABLE_OUT(x, v)      \
        out_dword(HWIO_GENI_S_IRQ_ENABLE_ADDR(x),v)
#define HWIO_GENI_S_IRQ_ENABLE_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_GENI_S_IRQ_ENABLE_ADDR(x),m,v,HWIO_GENI_S_IRQ_ENABLE_IN(x))
#define HWIO_GENI_S_IRQ_ENABLE_RX_FIFO_LAST_EN_BMSK                                        0x8000000
#define HWIO_GENI_S_IRQ_ENABLE_RX_FIFO_LAST_EN_SHFT                                             0x1b
#define HWIO_GENI_S_IRQ_ENABLE_RX_FIFO_WATERMARK_EN_BMSK                                   0x4000000
#define HWIO_GENI_S_IRQ_ENABLE_RX_FIFO_WATERMARK_EN_SHFT                                        0x1a
#define HWIO_GENI_S_IRQ_ENABLE_RX_FIFO_WR_ERR_EN_BMSK                                      0x2000000
#define HWIO_GENI_S_IRQ_ENABLE_RX_FIFO_WR_ERR_EN_SHFT                                           0x19
#define HWIO_GENI_S_IRQ_ENABLE_RX_FIFO_RD_ERR_EN_BMSK                                      0x1000000
#define HWIO_GENI_S_IRQ_ENABLE_RX_FIFO_RD_ERR_EN_SHFT                                           0x18
#define HWIO_GENI_S_IRQ_ENABLE_IO_DATA_ASSERT_EN_BMSK                                       0x800000
#define HWIO_GENI_S_IRQ_ENABLE_IO_DATA_ASSERT_EN_SHFT                                           0x17
#define HWIO_GENI_S_IRQ_ENABLE_IO_DATA_DEASSERT_EN_BMSK                                     0x400000
#define HWIO_GENI_S_IRQ_ENABLE_IO_DATA_DEASSERT_EN_SHFT                                         0x16
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_IRQ_5_EN_BMSK                                             0x4000
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_IRQ_5_EN_SHFT                                                0xe
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_IRQ_4_EN_BMSK                                             0x2000
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_IRQ_4_EN_SHFT                                                0xd
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_IRQ_3_EN_BMSK                                             0x1000
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_IRQ_3_EN_SHFT                                                0xc
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_IRQ_2_EN_BMSK                                              0x800
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_IRQ_2_EN_SHFT                                                0xb
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_IRQ_1_EN_BMSK                                              0x400
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_IRQ_1_EN_SHFT                                                0xa
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_IRQ_0_EN_BMSK                                              0x200
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_IRQ_0_EN_SHFT                                                0x9
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_SYNC_IRQ_0_EN_BMSK                                         0x100
#define HWIO_GENI_S_IRQ_ENABLE_S_GP_SYNC_IRQ_0_EN_SHFT                                           0x8
#define HWIO_GENI_S_IRQ_ENABLE_S_CMD_ABORT_EN_BMSK                                              0x20
#define HWIO_GENI_S_IRQ_ENABLE_S_CMD_ABORT_EN_SHFT                                               0x5
#define HWIO_GENI_S_IRQ_ENABLE_S_CMD_CANCEL_EN_BMSK                                             0x10
#define HWIO_GENI_S_IRQ_ENABLE_S_CMD_CANCEL_EN_SHFT                                              0x4
#define HWIO_GENI_S_IRQ_ENABLE_S_CMD_FAILURE_EN_BMSK                                             0x8
#define HWIO_GENI_S_IRQ_ENABLE_S_CMD_FAILURE_EN_SHFT                                             0x3
#define HWIO_GENI_S_IRQ_ENABLE_S_ILLEGAL_CMD_EN_BMSK                                             0x4
#define HWIO_GENI_S_IRQ_ENABLE_S_ILLEGAL_CMD_EN_SHFT                                             0x2
#define HWIO_GENI_S_IRQ_ENABLE_S_CMD_OVERRUN_EN_BMSK                                             0x2
#define HWIO_GENI_S_IRQ_ENABLE_S_CMD_OVERRUN_EN_SHFT                                             0x1
#define HWIO_GENI_S_IRQ_ENABLE_S_CMD_DONE_EN_BMSK                                                0x1
#define HWIO_GENI_S_IRQ_ENABLE_S_CMD_DONE_EN_SHFT                                                0x0

#define HWIO_GENI_S_IRQ_CLEAR_ADDR(x)                                                     ((x) + 0x00000048)
#define HWIO_GENI_S_IRQ_CLEAR_OFFS                                                        (0x00000048)
#define HWIO_GENI_S_IRQ_CLEAR_RMSK                                                         0xfc07f3f
#define HWIO_GENI_S_IRQ_CLEAR_OUT(x, v)      \
        out_dword(HWIO_GENI_S_IRQ_CLEAR_ADDR(x),v)
#define HWIO_GENI_S_IRQ_CLEAR_RX_FIFO_LAST_CLEAR_BMSK                                      0x8000000
#define HWIO_GENI_S_IRQ_CLEAR_RX_FIFO_LAST_CLEAR_SHFT                                           0x1b
#define HWIO_GENI_S_IRQ_CLEAR_RX_FIFO_WATERMARK_CLEAR_BMSK                                 0x4000000
#define HWIO_GENI_S_IRQ_CLEAR_RX_FIFO_WATERMARK_CLEAR_SHFT                                      0x1a
#define HWIO_GENI_S_IRQ_CLEAR_RX_FIFO_WR_ERR_CLEAR_BMSK                                    0x2000000
#define HWIO_GENI_S_IRQ_CLEAR_RX_FIFO_WR_ERR_CLEAR_SHFT                                         0x19
#define HWIO_GENI_S_IRQ_CLEAR_RX_FIFO_RD_ERR_CLEAR_BMSK                                    0x1000000
#define HWIO_GENI_S_IRQ_CLEAR_RX_FIFO_RD_ERR_CLEAR_SHFT                                         0x18
#define HWIO_GENI_S_IRQ_CLEAR_IO_DATA_ASSERT_CLEAR_BMSK                                     0x800000
#define HWIO_GENI_S_IRQ_CLEAR_IO_DATA_ASSERT_CLEAR_SHFT                                         0x17
#define HWIO_GENI_S_IRQ_CLEAR_IO_DATA_DEASSERT_CLEAR_BMSK                                   0x400000
#define HWIO_GENI_S_IRQ_CLEAR_IO_DATA_DEASSERT_CLEAR_SHFT                                       0x16
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_IRQ_5_CLEAR_BMSK                                           0x4000
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_IRQ_5_CLEAR_SHFT                                              0xe
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_IRQ_4_CLEAR_BMSK                                           0x2000
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_IRQ_4_CLEAR_SHFT                                              0xd
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_IRQ_3_CLEAR_BMSK                                           0x1000
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_IRQ_3_CLEAR_SHFT                                              0xc
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_IRQ_2_CLEAR_BMSK                                            0x800
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_IRQ_2_CLEAR_SHFT                                              0xb
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_IRQ_1_CLEAR_BMSK                                            0x400
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_IRQ_1_CLEAR_SHFT                                              0xa
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_IRQ_0_CLEAR_BMSK                                            0x200
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_IRQ_0_CLEAR_SHFT                                              0x9
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_SYNC_IRQ_0_CLEAR_BMSK                                       0x100
#define HWIO_GENI_S_IRQ_CLEAR_S_GP_SYNC_IRQ_0_CLEAR_SHFT                                         0x8
#define HWIO_GENI_S_IRQ_CLEAR_S_CMD_ABORT_CLEAR_BMSK                                            0x20
#define HWIO_GENI_S_IRQ_CLEAR_S_CMD_ABORT_CLEAR_SHFT                                             0x5
#define HWIO_GENI_S_IRQ_CLEAR_S_CMD_CANCEL_CLEAR_BMSK                                           0x10
#define HWIO_GENI_S_IRQ_CLEAR_S_CMD_CANCEL_CLEAR_SHFT                                            0x4
#define HWIO_GENI_S_IRQ_CLEAR_S_CMD_FAILURE_CLEAR_BMSK                                           0x8
#define HWIO_GENI_S_IRQ_CLEAR_S_CMD_FAILURE_CLEAR_SHFT                                           0x3
#define HWIO_GENI_S_IRQ_CLEAR_S_ILLEGAL_CMD_CLEAR_BMSK                                           0x4
#define HWIO_GENI_S_IRQ_CLEAR_S_ILLEGAL_CMD_CLEAR_SHFT                                           0x2
#define HWIO_GENI_S_IRQ_CLEAR_S_CMD_OVERRUN_CLEAR_BMSK                                           0x2
#define HWIO_GENI_S_IRQ_CLEAR_S_CMD_OVERRUN_CLEAR_SHFT                                           0x1
#define HWIO_GENI_S_IRQ_CLEAR_S_CMD_DONE_CLEAR_BMSK                                              0x1
#define HWIO_GENI_S_IRQ_CLEAR_S_CMD_DONE_CLEAR_SHFT                                              0x0

#define HWIO_GENI_TX_FIFOn_ADDR(base,n)                                                   ((base) + 0x00000100 + 0x4 * (n))
#define HWIO_GENI_TX_FIFOn_OFFS(base,n)                                                   (0x00000100 + 0x4 * (n))
#define HWIO_GENI_TX_FIFOn_RMSK                                                           0xffffffff
#define HWIO_GENI_TX_FIFOn_MAXn                                                                   15
#define HWIO_GENI_TX_FIFOn_OUTI(base,n,val)    \
        out_dword(HWIO_GENI_TX_FIFOn_ADDR(base,n),val)
#define HWIO_GENI_TX_FIFOn_TX_DATA_BMSK                                                   0xffffffff
#define HWIO_GENI_TX_FIFOn_TX_DATA_SHFT                                                          0x0

#define HWIO_GENI_RX_FIFOn_ADDR(base,n)                                                   ((base) + 0x00000180 + 0x4 * (n))
#define HWIO_GENI_RX_FIFOn_OFFS(base,n)                                                   (0x00000180 + 0x4 * (n))
#define HWIO_GENI_RX_FIFOn_RMSK                                                           0xffffffff
#define HWIO_GENI_RX_FIFOn_MAXn                                                                   15
#define HWIO_GENI_RX_FIFOn_INI(base,n)        \
        in_dword_masked(HWIO_GENI_RX_FIFOn_ADDR(base,n), HWIO_GENI_RX_FIFOn_RMSK)
#define HWIO_GENI_RX_FIFOn_INMI(base,n,mask)    \
        in_dword_masked(HWIO_GENI_RX_FIFOn_ADDR(base,n), mask)
#define HWIO_GENI_RX_FIFOn_RX_DATA_BMSK                                                   0xffffffff
#define HWIO_GENI_RX_FIFOn_RX_DATA_SHFT                                                          0x0

#define HWIO_GENI_RX_FIFO_STATUS_ADDR(x)                                                  ((x) + 0x00000204)
#define HWIO_GENI_RX_FIFO_STATUS_OFFS                                                     (0x00000204)
#define HWIO_GENI_RX_FIFO_STATUS_RMSK                                                     0xffffffff
#define HWIO_GENI_RX_FIFO_STATUS_IN(x)      \
        in_dword_masked(HWIO_GENI_RX_FIFO_STATUS_ADDR(x), HWIO_GENI_RX_FIFO_STATUS_RMSK)
#define HWIO_GENI_RX_FIFO_STATUS_INM(x, m)      \
        in_dword_masked(HWIO_GENI_RX_FIFO_STATUS_ADDR(x), m)
#define HWIO_GENI_RX_FIFO_STATUS_RX_LAST_BMSK                                             0x80000000
#define HWIO_GENI_RX_FIFO_STATUS_RX_LAST_SHFT                                                   0x1f
#define HWIO_GENI_RX_FIFO_STATUS_RX_LAST_BYTE_VALID_BMSK                                  0x70000000
#define HWIO_GENI_RX_FIFO_STATUS_RX_LAST_BYTE_VALID_SHFT                                        0x1c
#define HWIO_GENI_RX_FIFO_STATUS_RX_AFIFO_WC_BMSK                                          0xe000000
#define HWIO_GENI_RX_FIFO_STATUS_RX_AFIFO_WC_SHFT                                               0x19
#define HWIO_GENI_RX_FIFO_STATUS_RX_FIFO_WC_BMSK                                           0x1ffffff
#define HWIO_GENI_RX_FIFO_STATUS_RX_FIFO_WC_SHFT                                                 0x0

#define HWIO_GENI_TX_WATERMARK_REG_ADDR(x)                                                ((x) + 0x0000020c)
#define HWIO_GENI_TX_WATERMARK_REG_OFFS                                                   (0x0000020c)
#define HWIO_GENI_TX_WATERMARK_REG_RMSK                                                         0x3f
#define HWIO_GENI_TX_WATERMARK_REG_IN(x)      \
        in_dword_masked(HWIO_GENI_TX_WATERMARK_REG_ADDR(x), HWIO_GENI_TX_WATERMARK_REG_RMSK)
#define HWIO_GENI_TX_WATERMARK_REG_INM(x, m)      \
        in_dword_masked(HWIO_GENI_TX_WATERMARK_REG_ADDR(x), m)
#define HWIO_GENI_TX_WATERMARK_REG_OUT(x, v)      \
        out_dword(HWIO_GENI_TX_WATERMARK_REG_ADDR(x),v)
#define HWIO_GENI_TX_WATERMARK_REG_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_GENI_TX_WATERMARK_REG_ADDR(x),m,v,HWIO_GENI_TX_WATERMARK_REG_IN(x))
#define HWIO_GENI_TX_WATERMARK_REG_TX_WATERMARK_BMSK                                            0x3f
#define HWIO_GENI_TX_WATERMARK_REG_TX_WATERMARK_SHFT                                             0x0

#define HWIO_GENI_RX_WATERMARK_REG_ADDR(x)                                                ((x) + 0x00000210)
#define HWIO_GENI_RX_WATERMARK_REG_OFFS                                                   (0x00000210)
#define HWIO_GENI_RX_WATERMARK_REG_RMSK                                                         0x3f
#define HWIO_GENI_RX_WATERMARK_REG_IN(x)      \
        in_dword_masked(HWIO_GENI_RX_WATERMARK_REG_ADDR(x), HWIO_GENI_RX_WATERMARK_REG_RMSK)
#define HWIO_GENI_RX_WATERMARK_REG_INM(x, m)      \
        in_dword_masked(HWIO_GENI_RX_WATERMARK_REG_ADDR(x), m)
#define HWIO_GENI_RX_WATERMARK_REG_OUT(x, v)      \
        out_dword(HWIO_GENI_RX_WATERMARK_REG_ADDR(x),v)
#define HWIO_GENI_RX_WATERMARK_REG_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_GENI_RX_WATERMARK_REG_ADDR(x),m,v,HWIO_GENI_RX_WATERMARK_REG_IN(x))
#define HWIO_GENI_RX_WATERMARK_REG_RX_WATERMARK_BMSK                                            0x3f
#define HWIO_GENI_RX_WATERMARK_REG_RX_WATERMARK_SHFT                                             0x0

#define HWIO_GENI_RX_RFR_WATERMARK_REG_ADDR(x)                                            ((x) + 0x00000214)
#define HWIO_GENI_RX_RFR_WATERMARK_REG_OFFS                                               (0x00000214)
#define HWIO_GENI_RX_RFR_WATERMARK_REG_RMSK                                                     0x3f
#define HWIO_GENI_RX_RFR_WATERMARK_REG_IN(x)      \
        in_dword_masked(HWIO_GENI_RX_RFR_WATERMARK_REG_ADDR(x), HWIO_GENI_RX_RFR_WATERMARK_REG_RMSK)
#define HWIO_GENI_RX_RFR_WATERMARK_REG_INM(x, m)      \
        in_dword_masked(HWIO_GENI_RX_RFR_WATERMARK_REG_ADDR(x), m)
#define HWIO_GENI_RX_RFR_WATERMARK_REG_OUT(x, v)      \
        out_dword(HWIO_GENI_RX_RFR_WATERMARK_REG_ADDR(x),v)
#define HWIO_GENI_RX_RFR_WATERMARK_REG_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_GENI_RX_RFR_WATERMARK_REG_ADDR(x),m,v,HWIO_GENI_RX_RFR_WATERMARK_REG_IN(x))
#define HWIO_GENI_RX_RFR_WATERMARK_REG_RX_RFR_WATERMARK_BMSK                                    0x3f
#define HWIO_GENI_RX_RFR_WATERMARK_REG_RX_RFR_WATERMARK_SHFT                                     0x0

#define HWIO_UART_TX_TRANS_CFG_ADDR(x)                                                          ((x) + 0x0000015c)
#define HWIO_UART_TX_TRANS_CFG_OFFS                                                             (0x0000015c)
#define HWIO_UART_TX_TRANS_CFG_RMSK                                                                   0x3f
#define HWIO_UART_TX_TRANS_CFG_IN(x)      \
        in_dword_masked(HWIO_UART_TX_TRANS_CFG_ADDR(x), HWIO_UART_TX_TRANS_CFG_RMSK)
#define HWIO_UART_TX_TRANS_CFG_INM(x, m)      \
        in_dword_masked(HWIO_UART_TX_TRANS_CFG_ADDR(x), m)
#define HWIO_UART_TX_TRANS_CFG_OUT(x, v)      \
        out_dword(HWIO_UART_TX_TRANS_CFG_ADDR(x),v)
#define HWIO_UART_TX_TRANS_CFG_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_UART_TX_TRANS_CFG_ADDR(x),m,v,HWIO_UART_TX_TRANS_CFG_IN(x))
#define HWIO_UART_TX_TRANS_CFG_UART_NOT_USED_CFG3_BMSK                                                0x20
#define HWIO_UART_TX_TRANS_CFG_UART_NOT_USED_CFG3_SHFT                                                 0x5
#define HWIO_UART_TX_TRANS_CFG_UART_NOT_USED_CFG2_BMSK                                                0x10
#define HWIO_UART_TX_TRANS_CFG_UART_NOT_USED_CFG2_SHFT                                                 0x4
#define HWIO_UART_TX_TRANS_CFG_UART_NOT_USED_CFG1_BMSK                                                 0x8
#define HWIO_UART_TX_TRANS_CFG_UART_NOT_USED_CFG1_SHFT                                                 0x3
#define HWIO_UART_TX_TRANS_CFG_UART_NOT_USED_CFG_BMSK                                                  0x4
#define HWIO_UART_TX_TRANS_CFG_UART_NOT_USED_CFG_SHFT                                                  0x2
#define HWIO_UART_TX_TRANS_CFG_UART_CTS_MASK_BMSK                                                      0x2
#define HWIO_UART_TX_TRANS_CFG_UART_CTS_MASK_SHFT                                                      0x1
#define HWIO_UART_TX_TRANS_CFG_UART_PARITY_EN_BMSK                                                     0x1
#define HWIO_UART_TX_TRANS_CFG_UART_PARITY_EN_SHFT                                                     0x0

#define HWIO_UART_TX_WORD_LEN_ADDR(x)                                                           ((x) + 0x00000168)
#define HWIO_UART_TX_WORD_LEN_OFFS                                                              (0x00000168)
#define HWIO_UART_TX_WORD_LEN_RMSK                                                                   0x3ff
#define HWIO_UART_TX_WORD_LEN_IN(x)      \
        in_dword_masked(HWIO_UART_TX_WORD_LEN_ADDR(x), HWIO_UART_TX_WORD_LEN_RMSK)
#define HWIO_UART_TX_WORD_LEN_INM(x, m)      \
        in_dword_masked(HWIO_UART_TX_WORD_LEN_ADDR(x), m)
#define HWIO_UART_TX_WORD_LEN_OUT(x, v)      \
        out_dword(HWIO_UART_TX_WORD_LEN_ADDR(x),v)
#define HWIO_UART_TX_WORD_LEN_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_UART_TX_WORD_LEN_ADDR(x),m,v,HWIO_UART_TX_WORD_LEN_IN(x))
#define HWIO_UART_TX_WORD_LEN_UART_TX_WORD_LEN_BMSK                                                  0x3ff
#define HWIO_UART_TX_WORD_LEN_UART_TX_WORD_LEN_SHFT                                                    0x0

#define HWIO_UART_TX_STOP_BIT_LEN_ADDR(x)                                                       ((x) + 0x0000016c)
#define HWIO_UART_TX_STOP_BIT_LEN_OFFS                                                          (0x0000016c)
#define HWIO_UART_TX_STOP_BIT_LEN_RMSK                                                            0xffffff
#define HWIO_UART_TX_STOP_BIT_LEN_IN(x)      \
        in_dword_masked(HWIO_UART_TX_STOP_BIT_LEN_ADDR(x), HWIO_UART_TX_STOP_BIT_LEN_RMSK)
#define HWIO_UART_TX_STOP_BIT_LEN_INM(x, m)      \
        in_dword_masked(HWIO_UART_TX_STOP_BIT_LEN_ADDR(x), m)
#define HWIO_UART_TX_STOP_BIT_LEN_OUT(x, v)      \
        out_dword(HWIO_UART_TX_STOP_BIT_LEN_ADDR(x),v)
#define HWIO_UART_TX_STOP_BIT_LEN_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_UART_TX_STOP_BIT_LEN_ADDR(x),m,v,HWIO_UART_TX_STOP_BIT_LEN_IN(x))
#define HWIO_UART_TX_STOP_BIT_LEN_UART_TX_STOP_BIT_LEN_BMSK                                       0xffffff
#define HWIO_UART_TX_STOP_BIT_LEN_UART_TX_STOP_BIT_LEN_SHFT                                            0x0

#define HWIO_UART_TX_TRANS_LEN_ADDR(x)                                                          ((x) + 0x00000170)
#define HWIO_UART_TX_TRANS_LEN_OFFS                                                             (0x00000170)
#define HWIO_UART_TX_TRANS_LEN_RMSK                                                               0xffffff
#define HWIO_UART_TX_TRANS_LEN_IN(x)      \
        in_dword_masked(HWIO_UART_TX_TRANS_LEN_ADDR(x), HWIO_UART_TX_TRANS_LEN_RMSK)
#define HWIO_UART_TX_TRANS_LEN_INM(x, m)      \
        in_dword_masked(HWIO_UART_TX_TRANS_LEN_ADDR(x), m)
#define HWIO_UART_TX_TRANS_LEN_OUT(x, v)      \
        out_dword(HWIO_UART_TX_TRANS_LEN_ADDR(x),v)
#define HWIO_UART_TX_TRANS_LEN_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_UART_TX_TRANS_LEN_ADDR(x),m,v,HWIO_UART_TX_TRANS_LEN_IN(x))
#define HWIO_UART_TX_TRANS_LEN_UART_TX_TRANS_LEN_BMSK                                             0xffffff
#define HWIO_UART_TX_TRANS_LEN_UART_TX_TRANS_LEN_SHFT                                                  0x0

#define HWIO_UART_RX_TRANS_CFG_ADDR(x)                                                          ((x) + 0x00000180)
#define HWIO_UART_RX_TRANS_CFG_OFFS                                                             (0x00000180)
#define HWIO_UART_RX_TRANS_CFG_RMSK                                                                   0x3f
#define HWIO_UART_RX_TRANS_CFG_IN(x)      \
        in_dword_masked(HWIO_UART_RX_TRANS_CFG_ADDR(x), HWIO_UART_RX_TRANS_CFG_RMSK)
#define HWIO_UART_RX_TRANS_CFG_INM(x, m)      \
        in_dword_masked(HWIO_UART_RX_TRANS_CFG_ADDR(x), m)
#define HWIO_UART_RX_TRANS_CFG_OUT(x, v)      \
        out_dword(HWIO_UART_RX_TRANS_CFG_ADDR(x),v)
#define HWIO_UART_RX_TRANS_CFG_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_UART_RX_TRANS_CFG_ADDR(x),m,v,HWIO_UART_RX_TRANS_CFG_IN(x))
#define HWIO_UART_RX_TRANS_CFG_UART_NOT_USED_CFG3_BMSK                                                0x20
#define HWIO_UART_RX_TRANS_CFG_UART_NOT_USED_CFG3_SHFT                                                 0x5
#define HWIO_UART_RX_TRANS_CFG_UART_NOT_USED_CFG2_BMSK                                                0x10
#define HWIO_UART_RX_TRANS_CFG_UART_NOT_USED_CFG2_SHFT                                                 0x4
#define HWIO_UART_RX_TRANS_CFG_UART_PARITY_EN_BMSK                                                     0x8
#define HWIO_UART_RX_TRANS_CFG_UART_PARITY_EN_SHFT                                                     0x3
#define HWIO_UART_RX_TRANS_CFG_UART_RX_INSERT_STATUS_BIT_BMSK                                          0x4
#define HWIO_UART_RX_TRANS_CFG_UART_RX_INSERT_STATUS_BIT_SHFT                                          0x2
#define HWIO_UART_RX_TRANS_CFG_UART_NOT_USED_CFG1_BMSK                                                 0x2
#define HWIO_UART_RX_TRANS_CFG_UART_NOT_USED_CFG1_SHFT                                                 0x1
#define HWIO_UART_RX_TRANS_CFG_UART_NOT_USED_CFG0_BMSK                                                 0x1
#define HWIO_UART_RX_TRANS_CFG_UART_NOT_USED_CFG0_SHFT                                                 0x0

#define HWIO_UART_RX_WORD_LEN_ADDR(x)                                                           ((x) + 0x0000018c)
#define HWIO_UART_RX_WORD_LEN_OFFS                                                              (0x0000018c)
#define HWIO_UART_RX_WORD_LEN_RMSK                                                                   0x3ff
#define HWIO_UART_RX_WORD_LEN_IN(x)      \
        in_dword_masked(HWIO_UART_RX_WORD_LEN_ADDR(x), HWIO_UART_RX_WORD_LEN_RMSK)
#define HWIO_UART_RX_WORD_LEN_INM(x, m)      \
        in_dword_masked(HWIO_UART_RX_WORD_LEN_ADDR(x), m)
#define HWIO_UART_RX_WORD_LEN_OUT(x, v)      \
        out_dword(HWIO_UART_RX_WORD_LEN_ADDR(x),v)
#define HWIO_UART_RX_WORD_LEN_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_UART_RX_WORD_LEN_ADDR(x),m,v,HWIO_UART_RX_WORD_LEN_IN(x))
#define HWIO_UART_RX_WORD_LEN_UART_RX_WORD_LEN_BMSK                                                  0x3ff
#define HWIO_UART_RX_WORD_LEN_UART_RX_WORD_LEN_SHFT                                                    0x0

#define HWIO_UART_RX_STALE_CNT_ADDR(x)                                                          ((x) + 0x00000194)
#define HWIO_UART_RX_STALE_CNT_OFFS                                                             (0x00000194)
#define HWIO_UART_RX_STALE_CNT_RMSK                                                               0xffffff
#define HWIO_UART_RX_STALE_CNT_IN(x)      \
        in_dword_masked(HWIO_UART_RX_STALE_CNT_ADDR(x), HWIO_UART_RX_STALE_CNT_RMSK)
#define HWIO_UART_RX_STALE_CNT_INM(x, m)      \
        in_dword_masked(HWIO_UART_RX_STALE_CNT_ADDR(x), m)
#define HWIO_UART_RX_STALE_CNT_OUT(x, v)      \
        out_dword(HWIO_UART_RX_STALE_CNT_ADDR(x),v)
#define HWIO_UART_RX_STALE_CNT_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_UART_RX_STALE_CNT_ADDR(x),m,v,HWIO_UART_RX_STALE_CNT_IN(x))
#define HWIO_UART_RX_STALE_CNT_UART_RX_STALE_CNT_BMSK                                             0xffffff
#define HWIO_UART_RX_STALE_CNT_UART_RX_STALE_CNT_SHFT                                                  0x0

#define HWIO_UART_TX_PARITY_CFG_ADDR(x)                                                         ((x) + 0x000001a4)
#define HWIO_UART_TX_PARITY_CFG_OFFS                                                            (0x000001a4)
#define HWIO_UART_TX_PARITY_CFG_RMSK                                                                  0x1f
#define HWIO_UART_TX_PARITY_CFG_IN(x)      \
        in_dword_masked(HWIO_UART_TX_PARITY_CFG_ADDR(x), HWIO_UART_TX_PARITY_CFG_RMSK)
#define HWIO_UART_TX_PARITY_CFG_INM(x, m)      \
        in_dword_masked(HWIO_UART_TX_PARITY_CFG_ADDR(x), m)
#define HWIO_UART_TX_PARITY_CFG_OUT(x, v)      \
        out_dword(HWIO_UART_TX_PARITY_CFG_ADDR(x),v)
#define HWIO_UART_TX_PARITY_CFG_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_UART_TX_PARITY_CFG_ADDR(x),m,v,HWIO_UART_TX_PARITY_CFG_IN(x))
#define HWIO_UART_TX_PARITY_CFG_UART_NOT_USED_CFG1_BMSK                                               0x10
#define HWIO_UART_TX_PARITY_CFG_UART_NOT_USED_CFG1_SHFT                                                0x4
#define HWIO_UART_TX_PARITY_CFG_UART_NOT_USED_CFG0_BMSK                                                0x8
#define HWIO_UART_TX_PARITY_CFG_UART_NOT_USED_CFG0_SHFT                                                0x3
#define HWIO_UART_TX_PARITY_CFG_TX_PAR_MODE_BMSK                                                       0x6
#define HWIO_UART_TX_PARITY_CFG_TX_PAR_MODE_SHFT                                                       0x1
#define HWIO_UART_TX_PARITY_CFG_TX_PAR_CALC_EN_BMSK                                                    0x1
#define HWIO_UART_TX_PARITY_CFG_TX_PAR_CALC_EN_SHFT                                                    0x0

#define HWIO_UART_RX_PARITY_CFG_ADDR(x)                                                         ((x) + 0x000001a8)
#define HWIO_UART_RX_PARITY_CFG_OFFS                                                            (0x000001a8)
#define HWIO_UART_RX_PARITY_CFG_RMSK                                                                  0x1f
#define HWIO_UART_RX_PARITY_CFG_IN(x)      \
        in_dword_masked(HWIO_UART_RX_PARITY_CFG_ADDR(x), HWIO_UART_RX_PARITY_CFG_RMSK)
#define HWIO_UART_RX_PARITY_CFG_INM(x, m)      \
        in_dword_masked(HWIO_UART_RX_PARITY_CFG_ADDR(x), m)
#define HWIO_UART_RX_PARITY_CFG_OUT(x, v)      \
        out_dword(HWIO_UART_RX_PARITY_CFG_ADDR(x),v)
#define HWIO_UART_RX_PARITY_CFG_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_UART_RX_PARITY_CFG_ADDR(x),m,v,HWIO_UART_RX_PARITY_CFG_IN(x))
#define HWIO_UART_RX_PARITY_CFG_UART_NOT_USED_CFG1_BMSK                                               0x10
#define HWIO_UART_RX_PARITY_CFG_UART_NOT_USED_CFG1_SHFT                                                0x4
#define HWIO_UART_RX_PARITY_CFG_UART_NOT_USED_CFG0_BMSK                                                0x8
#define HWIO_UART_RX_PARITY_CFG_UART_NOT_USED_CFG0_SHFT                                                0x3
#define HWIO_UART_RX_PARITY_CFG_RX_PAR_MODE_BMSK                                                       0x6
#define HWIO_UART_RX_PARITY_CFG_RX_PAR_MODE_SHFT                                                       0x1
#define HWIO_UART_RX_PARITY_CFG_RX_PAR_CALC_EN_BMSK                                                    0x1
#define HWIO_UART_RX_PARITY_CFG_RX_PAR_CALC_EN_SHFT                                                    0x0


#define HWIO_GENI_DMA_MODE_EN_ADDR(x)                                                           ((x) + 0x00000158)
#define HWIO_GENI_DMA_MODE_EN_OFFS                                                              (0x00000158)
#define HWIO_GENI_DMA_MODE_EN_RMSK                                                                     0x1
#define HWIO_GENI_DMA_MODE_EN_IN(x)      \
        in_dword_masked(HWIO_GENI_DMA_MODE_EN_ADDR(x), HWIO_GENI_DMA_MODE_EN_RMSK)
#define HWIO_GENI_DMA_MODE_EN_INM(x, m)      \
        in_dword_masked(HWIO_GENI_DMA_MODE_EN_ADDR(x), m)
#define HWIO_GENI_DMA_MODE_EN_OUT(x, v)      \
        out_dword(HWIO_GENI_DMA_MODE_EN_ADDR(x),v)
#define HWIO_GENI_DMA_MODE_EN_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_GENI_DMA_MODE_EN_ADDR(x),m,v,HWIO_GENI_DMA_MODE_EN_IN(x))
#define HWIO_GENI_DMA_MODE_EN_GENI_DMA_MODE_EN_BMSK                                                    0x1
#define HWIO_GENI_DMA_MODE_EN_GENI_DMA_MODE_EN_SHFT                                                    0x0

#define HWIO_SE_IRQ_EN_ADDR(x)                                                           ((x) + 0x0000021c)
#define HWIO_SE_IRQ_EN_OFFS                                                              (0x0000021c)
#define HWIO_SE_IRQ_EN_RMSK                                                                     0xf
#define HWIO_SE_IRQ_EN_IN(x)      \
        in_dword_masked(HWIO_SE_IRQ_EN_ADDR(x), HWIO_SE_IRQ_EN_RMSK)
#define HWIO_SE_IRQ_EN_INM(x, m)      \
        in_dword_masked(HWIO_SE_IRQ_EN_ADDR(x), m)
#define HWIO_SE_IRQ_EN_OUT(x, v)      \
        out_dword(HWIO_SE_IRQ_EN_ADDR(x),v)
#define HWIO_SE_IRQ_EN_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_SE_IRQ_EN_ADDR(x),m,v,HWIO_SE_IRQ_EN_IN(x))
#define HWIO_SE_IRQ_EN_GENI_S_IRQ_EN_BMSK                                                       0x8
#define HWIO_SE_IRQ_EN_GENI_S_IRQ_EN_SHFT                                                       0x3
#define HWIO_SE_IRQ_EN_GENI_M_IRQ_EN_BMSK                                                       0x4
#define HWIO_SE_IRQ_EN_GENI_M_IRQ_EN_SHFT                                                       0x2
#define HWIO_SE_IRQ_EN_DMA_TX_IRQ_EN_BMSK                                                       0x2
#define HWIO_SE_IRQ_EN_DMA_TX_IRQ_EN_SHFT                                                       0x1
#define HWIO_SE_IRQ_EN_DMA_RX_IRQ_EN_BMSK                                                       0x1
#define HWIO_SE_IRQ_EN_DMA_RX_IRQ_EN_SHFT                                                       0x0

#define HWIO_SE_HW_PARAM_0_ADDR(x)                                                       ((x) + 0x00000224)
#define HWIO_SE_HW_PARAM_0_OFFS                                                          (0x00000224)
#define HWIO_SE_HW_PARAM_0_RMSK                                                          0x3f3f79ff
#define HWIO_SE_HW_PARAM_0_IN(x)      \
        in_dword_masked(HWIO_SE_HW_PARAM_0_ADDR(x), HWIO_SE_HW_PARAM_0_RMSK)
#define HWIO_SE_HW_PARAM_0_INM(x, m)      \
        in_dword_masked(HWIO_SE_HW_PARAM_0_ADDR(x), m)
#define HWIO_SE_HW_PARAM_0_TX_FIFO_WIDTH_BMSK                                            0x3f000000
#define HWIO_SE_HW_PARAM_0_TX_FIFO_WIDTH_SHFT                                                  0x18
#define HWIO_SE_HW_PARAM_0_TX_FIFO_DEPTH_BMSK                                              0x3f0000
#define HWIO_SE_HW_PARAM_0_TX_FIFO_DEPTH_SHFT                                                  0x10
#define HWIO_SE_HW_PARAM_0_TX_ASYNC_FIFO_DEPTH_BMSK                                          0x7000
#define HWIO_SE_HW_PARAM_0_TX_ASYNC_FIFO_DEPTH_SHFT                                             0xc
#define HWIO_SE_HW_PARAM_0_TX_FIFO_EN_BMSK                                                    0x800
#define HWIO_SE_HW_PARAM_0_TX_FIFO_EN_SHFT                                                      0xb
#define HWIO_SE_HW_PARAM_0_GEN_I3C_BMSK                                                       0x100
#define HWIO_SE_HW_PARAM_0_GEN_I3C_SHFT                                                         0x8
#define HWIO_SE_HW_PARAM_0_GEN_PROG_ROM_BMSK                                                   0x80
#define HWIO_SE_HW_PARAM_0_GEN_PROG_ROM_SHFT                                                    0x7
#define HWIO_SE_HW_PARAM_0_GEN_SCND_SEQUENCER_BMSK                                             0x40
#define HWIO_SE_HW_PARAM_0_GEN_SCND_SEQUENCER_SHFT                                              0x6
#define HWIO_SE_HW_PARAM_0_AHB_M_ADDR_W_BMSK                                                   0x3f
#define HWIO_SE_HW_PARAM_0_AHB_M_ADDR_W_SHFT                                                    0x0

#define HWIO_SE_HW_PARAM_1_ADDR(x)                                                       ((x) + 0x00000228)
#define HWIO_SE_HW_PARAM_1_OFFS                                                          (0x00000228)
#define HWIO_SE_HW_PARAM_1_RMSK                                                          0x3f3f7fff
#define HWIO_SE_HW_PARAM_1_IN(x)      \
        in_dword_masked(HWIO_SE_HW_PARAM_1_ADDR(x), HWIO_SE_HW_PARAM_1_RMSK)
#define HWIO_SE_HW_PARAM_1_INM(x, m)      \
        in_dword_masked(HWIO_SE_HW_PARAM_1_ADDR(x), m)
#define HWIO_SE_HW_PARAM_1_RX_FIFO_WIDTH_BMSK                                            0x3f000000
#define HWIO_SE_HW_PARAM_1_RX_FIFO_WIDTH_SHFT                                                  0x18
#define HWIO_SE_HW_PARAM_1_RX_FIFO_DEPTH_BMSK                                              0x3f0000
#define HWIO_SE_HW_PARAM_1_RX_FIFO_DEPTH_SHFT                                                  0x10
#define HWIO_SE_HW_PARAM_1_RX_ASYNC_FIFO_DEPTH_BMSK                                          0x7000
#define HWIO_SE_HW_PARAM_1_RX_ASYNC_FIFO_DEPTH_SHFT                                             0xc
#define HWIO_SE_HW_PARAM_1_RX_FIFO_EN_BMSK                                                    0x800
#define HWIO_SE_HW_PARAM_1_RX_FIFO_EN_SHFT                                                      0xb
#define HWIO_SE_HW_PARAM_1_PROG_RAM_MEM_TYPE_BMSK                                             0x600
#define HWIO_SE_HW_PARAM_1_PROG_RAM_MEM_TYPE_SHFT                                               0x9
#define HWIO_SE_HW_PARAM_1_PROG_REG_ARRAY_DEPTH_BMSK                                          0x1ff
#define HWIO_SE_HW_PARAM_1_PROG_REG_ARRAY_DEPTH_SHFT                                            0x0

#define HWIO_DMA_GENERAL_CFG_ADDR(x)                                                     ((x) + 0x00000230)
#define HWIO_DMA_GENERAL_CFG_OFFS                                                        (0x00000230)
#define HWIO_DMA_GENERAL_CFG_RMSK                                                             0x1ff
#define HWIO_DMA_GENERAL_CFG_IN(x)      \
        in_dword_masked(HWIO_DMA_GENERAL_CFG_ADDR(x), HWIO_DMA_GENERAL_CFG_RMSK)
#define HWIO_DMA_GENERAL_CFG_INM(x, m)      \
        in_dword_masked(HWIO_DMA_GENERAL_CFG_ADDR(x), m)
#define HWIO_DMA_GENERAL_CFG_OUT(x, v)      \
        out_dword(HWIO_DMA_GENERAL_CFG_ADDR(x),v)
#define HWIO_DMA_GENERAL_CFG_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_DMA_GENERAL_CFG_ADDR(x),m,v,HWIO_DMA_GENERAL_CFG_IN(x))
#define HWIO_DMA_GENERAL_CFG_RX_DMA_IRQ_DELAY_BMSK                                            0x1c0
#define HWIO_DMA_GENERAL_CFG_RX_DMA_IRQ_DELAY_SHFT                                              0x6
#define HWIO_DMA_GENERAL_CFG_TX_DMA_ZERO_PADDING_EN_BMSK                                       0x20
#define HWIO_DMA_GENERAL_CFG_TX_DMA_ZERO_PADDING_EN_SHFT                                        0x5
#define HWIO_DMA_GENERAL_CFG_AHB_SEC_SLV_CLK_CGC_ON_BMSK                                        0x8
#define HWIO_DMA_GENERAL_CFG_AHB_SEC_SLV_CLK_CGC_ON_SHFT                                        0x3
#define HWIO_DMA_GENERAL_CFG_DMA_AHB_SLV_CLK_CGC_ON_BMSK                                        0x4
#define HWIO_DMA_GENERAL_CFG_DMA_AHB_SLV_CLK_CGC_ON_SHFT                                        0x2
#define HWIO_DMA_GENERAL_CFG_DMA_TX_CLK_CGC_ON_BMSK                                             0x2
#define HWIO_DMA_GENERAL_CFG_DMA_TX_CLK_CGC_ON_SHFT                                             0x1
#define HWIO_DMA_GENERAL_CFG_DMA_RX_CLK_CGC_ON_BMSK                                             0x1
#define HWIO_DMA_GENERAL_CFG_DMA_RX_CLK_CGC_ON_SHFT                                             0x0

#define HWIO_SE_GSI_EVENT_EN_ADDR(x)                                                     ((x) + 0x00000218)
#define HWIO_SE_GSI_EVENT_EN_OFFS                                                        (0x00000218)
#define HWIO_SE_GSI_EVENT_EN_RMSK                                                               0xf
#define HWIO_SE_GSI_EVENT_EN_IN(x)      \
        in_dword_masked(HWIO_SE_GSI_EVENT_EN_ADDR(x), HWIO_SE_GSI_EVENT_EN_RMSK)
#define HWIO_SE_GSI_EVENT_EN_INM(x, m)      \
        in_dword_masked(HWIO_SE_GSI_EVENT_EN_ADDR(x), m)
#define HWIO_SE_GSI_EVENT_EN_OUT(x, v)      \
        out_dword(HWIO_SE_GSI_EVENT_EN_ADDR(x),v)
#define HWIO_SE_GSI_EVENT_EN_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_SE_GSI_EVENT_EN_ADDR(x),m,v,HWIO_SE_GSI_EVENT_EN_IN(x))
#define HWIO_SE_GSI_EVENT_EN_GENI_S_EVENT_EN_BMSK                                               0x8
#define HWIO_SE_GSI_EVENT_EN_GENI_S_EVENT_EN_SHFT                                               0x3
#define HWIO_SE_GSI_EVENT_EN_GENI_M_EVENT_EN_BMSK                                               0x4
#define HWIO_SE_GSI_EVENT_EN_GENI_M_EVENT_EN_SHFT                                               0x2
#define HWIO_SE_GSI_EVENT_EN_DMA_TX_EVENT_EN_BMSK                                               0x2
#define HWIO_SE_GSI_EVENT_EN_DMA_TX_EVENT_EN_SHFT                                               0x1
#define HWIO_SE_GSI_EVENT_EN_DMA_RX_EVENT_EN_BMSK                                               0x1

#define HWIO_QUPV3_SE_AHB_M_CFG_ADDR(x)                                                  ((x) + 0x00000118)
#define HWIO_QUPV3_SE_AHB_M_CFG_OFFS                                                     (0x00000118)
#define HWIO_QUPV3_SE_AHB_M_CFG_RMSK                                                            0x1
#define HWIO_QUPV3_SE_AHB_M_CFG_IN(x)      \
        in_dword_masked(HWIO_QUPV3_SE_AHB_M_CFG_ADDR(x), HWIO_QUPV3_SE_AHB_M_CFG_RMSK)
#define HWIO_QUPV3_SE_AHB_M_CFG_INM(x, m)      \
        in_dword_masked(HWIO_QUPV3_SE_AHB_M_CFG_ADDR(x), m)
#define HWIO_QUPV3_SE_AHB_M_CFG_OUT(x, v)      \
        out_dword(HWIO_QUPV3_SE_AHB_M_CFG_ADDR(x),v)
#define HWIO_QUPV3_SE_AHB_M_CFG_OUTM(x,m,v) \
        out_dword_masked_ns(HWIO_QUPV3_SE_AHB_M_CFG_ADDR(x),m,v,HWIO_QUPV3_SE_AHB_M_CFG_IN(x))
#define HWIO_QUPV3_SE_AHB_M_CFG_AHB_M_CLK_CGC_ON_BMSK                                           0x1
#define HWIO_QUPV3_SE_AHB_M_CFG_AHB_M_CLK_CGC_ON_SHFT                                           0x0

#define MAX_RX_FIFO_SIZE 128
#define MAX_RETRIES 0x100000

// --------------------------------------------------------------------- Macros

#define UART_DM_READ_REG(addr, offset)          \
    MmioRead32((UINT32)((UINT8)addr + offset))

#define UART_DM_WRITE_REG(addr, offset, val)    \
    MmioWrite32((UINT32)((UINT8)addr + offset), val)

// -------------------------------------------------------------------- Globals

//
// This structure indicates the local stored data read/write from/to FIFO HW.
//

typedef struct _FIFO_TX_BLOCK {
    UINT8 FifoBuffer[MAX_RX_FIFO_SIZE];
    UINT32 AvailableBytes;
    UINT8* PtrToFifoBuffer;
} FIFO_TX_BLOCK, *PFIFO_TX_BLOCK;