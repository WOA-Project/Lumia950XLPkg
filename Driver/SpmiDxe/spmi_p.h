/* Copyright (c) 2012,2014-2015 The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
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

#ifndef __SPMI_PRIVATE_H
#define __SPMI_PRIVATE_H

#define PMICV2_ARB_CORE_REG_BASE               (SPMI_BASE + 0x00400000)
#define PMICV2_ARB_OBS_CORE_REG_BASE           (SPMI_BASE + 0x00C00000)
#define PMICV2_ARB_CHNLn_CONFIG(x)             (PMICV2_ARB_CORE_REG_BASE + 0x00000004 + (x) * 0x8000)
#define PMICV2_ARB_OBS_CHNLn_CONFIG(x)         (PMICV2_ARB_OBS_CORE_REG_BASE + 0x00000004 + (x) * 0x8000)
#define PMICV2_ARB_CHNLn_STATUS(x)             (PMICV2_ARB_CORE_REG_BASE + 0x00000008 + (x) * 0x8000)
#define PMICV2_ARB_OBS_CHNLn_STATUS(x)         (PMICV2_ARB_OBS_CORE_REG_BASE + 0x00000008 + (x) * 0x8000)
#define PMICV2_ARB_CHNLn_WDATA(x, n)           (PMICV2_ARB_CORE_REG_BASE + 0x00000010 + (x) * 0x8000 + (n) * 4)
#define PMICV2_ARB_CHNLn_RDATA(x,n)            (PMICV2_ARB_CORE_REG_BASE + 0x00000018 + (x) * 0x8000 + (n) * 4)
#define PMICV2_ARB_OBS_CHNLn_RDATA(x,n)        (PMICV2_ARB_OBS_CORE_REG_BASE + 0x00000018 + (x) * 0x8000 + (n) * 4)

#define PMICV2_ARB_REG_CHLN(n)                 (PMIC_ARB_CORE + 0x00000800 + 0x4 * (n))
#define PMICV2_ARB_CHNLn_CMD0(x)               (PMICV2_ARB_CORE_REG_BASE + (x) * 0x8000)
#define PMICV2_ARB_OBS_CHNLn_CMD0(x)           (PMICV2_ARB_OBS_CORE_REG_BASE + (x) * 0x8000)

#define SPMIV1_MSM8974_MASTER_ID               0

#define SPMIV1_GENI_REG(x)                     (SPMI_GENI_BASE + (x))
#define SPMIV1_GENI_CFG_REG_BASE               SPMIV1_GENI_REG(0x100)
#define SPMIV1_GENI_CFG_RAM_BASE               SPMIV1_GENI_REG(0x200)

#define SPMIV1_GENI_CFG_REGn(x)                (SPMIV1_GENI_CFG_REG_BASE + 4 * (x))
#define SPMIV1_GENI_CFG_RAM_REGn(x)            (SPMIV1_GENI_CFG_RAM_BASE + 4 * (x))

#define SPMIV1_GENI_CLK_CTRL_REG               SPMIV1_GENI_REG(0x00)
#define SPMIV1_GENI_OUTPUT_CTRL_REG            SPMIV1_GENI_REG(0x10)
#define SPMIV1_GENI_FORCE_DEFAULT_REG          SPMIV1_GENI_REG(0x0C)
#define SPMIV1_MID_REG                         SPMIV1_GENI_REG(0xF00)
#define SPMIV1_CFG_REG                         SPMIV1_GENI_REG(0xF04)
#define SPMIV1_SEC_DISABLE_REG                 SPMIV1_GENI_REG(0xF08)

#define SPMIV1_GENI_IRQ_ENABLE                 (SPMI_GENI_BASE + 0x24)
#define SPMIV1_GENI_IRQ_CLEAR                  (SPMI_GENI_BASE + 0x28)
#define SPMIV1_GENI_TX_FIFO_BASE               (SPMI_GENI_BASE + 0x40)
#define SPMIV1_GENI_RX_FIFO_BASE               (SPMI_GENI_BASE + 0x80)

#define SPMIV1_GENI_TX_FIFOn(x)                (SPMIV1_GENI_TX_FIFO_BASE + 4 * (x))
#define SPMIV1_GENI_RX_FIFOn(x)                (SPMIV1_GENI_RX_FIFO_BASE + 4 * (x))

#define PMICV1_ARB_CHNLn_CMD0(x)               (SPMI_BASE + 0xF800 + (x) * 0x80)

#define PMICV1_ARB_CHNLn_CONFIG(x)             (SPMI_BASE + 0xF804 + (x) * 0x80)
#define PMICV1_ARB_CHNLn_STATUS(x)             (SPMI_BASE + 0xF808 + (x) * 0x80)
#define PMICV1_ARB_CHNLn_WDATA(x, n)           (SPMI_BASE + 0xF810 + \
	(x) * 0x80 + (n) * 4)
#define PMICV1_ARB_CHNLn_RDATA(x,n)            (SPMI_BASE + 0xF818 + \
	(x) * 0x80 + (n) * 4)

#define PMIC_ARB_CMD_OPCODE_SHIFT            27
#define PMIC_ARB_CMD_PRIORITY_SHIFT          26
#define PMIC_ARB_CMD_SLAVE_ID_SHIFT          20
#define PMIC_ARB_CMD_ADDR_SHIFT              12
#define PMIC_ARB_CMD_ADDR_OFFSET_SHIFT       4
#define PMIC_ARB_CMD_BYTE_CNT_SHIFT          0

/* SPMI Commands */
#define SPMI_CMD_EXT_REG_WRTIE_LONG          0x00
#define SPMI_CMD_EXT_REG_READ_LONG           0x01
#define SPMI_CMD_EXT_REG_READ_LONG_DELAYED   0x02
#define SPMI_CMD_TRANSFER_BUS_OWNERSHIP      0x03

/* The commands below are not yet supported */
#define SPMI_CMD_RESET                       0x04
#define SPMI_CMD_SLEEP                       0x05
#define SPMI_CMD_SHUTDOWN                    0x06
#define SPMI_CMD_WAKEUP                      0x07
#define SPMI_CMD_EXT_REG_WRITE               0x08
#define SPMI_CMD_EXT_REG_READ                0x09
#define SPMI_CMD_REG_WRITE                   0x0A
#define SPMI_CMD_REG_READ                    0x0B
#define SPMI_CMD_REG_0_WRITE                 0x0C
#define SPMI_CMD_AUTH                        0x0D
#define SPMI_CMD_MASTER_WRITE                0x0E
#define SPMI_CMD_MASTER_READ                 0x0F
#define SPMI_CMD_DEV_DESC_BLK_MASTER_READ    0x10
#define SPMI_CMD_DEV_DESC_BLK_SLAVE_READ     0x11

#define PMIC_ARB_SPMI_HW_VERSION             (SPMI_BASE + 0xF000)

enum spmi_geni_cmd_return_value{
	SPMI_CMD_DONE,
	SMPI_CMD_DENIED,
	SPMI_CMD_FAILURE,
	SPMI_ILLEGAL_CMD,
	SPMI_CMD_OVERRUN = 6,
	SPMI_TX_FIFO_RD_ERR,
	SPMI_TX_FIFO_WR_ERR,
	SPMI_RX_FIFO_RD_ERR,
	SPMI_RX_FIFO_WR_ERR
};

enum pmic_arb_chnl_return_values{
	PMIC_ARB_CMD_DONE,
	PMIC_ARB_CMD_FAILURE,
	PMIC_ARB_CMD_DENIED,
	PMIC_ARB_CMD_DROPPED,
};

int spmi_init(void);
unsigned int pmic_arb_write_cmd(struct pmic_arb_cmd *cmd,
	struct pmic_arb_param *param);
unsigned int pmic_arb_read_cmd(struct pmic_arb_cmd *cmd,
	struct pmic_arb_param *param);

#endif
