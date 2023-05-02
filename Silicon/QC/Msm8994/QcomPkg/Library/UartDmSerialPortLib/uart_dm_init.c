/* Copyright (c) 2010, The Linux Foundation. All rights reserved.

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

#include <Base.h>

#include <Library/LKEnvLib.h>

#include <Chipset/gsbi.h>
#include <Library/QcomPlatformUartDmLib.h>

#include "uartdm_p.h"

/* Static Function Prototype Declarations */
static unsigned int msm_boot_uart_dm_init(uint32_t base);
static unsigned int msm_boot_uart_dm_init_rx_transfer(uint32_t base);
static unsigned int msm_boot_uart_dm_reset(uint32_t base);

/*
 * Reset the UART
 */
static unsigned int msm_boot_uart_dm_reset(uint32_t base)
{
  writel(MSM_BOOT_UART_DM_CMD_RESET_RX, MSM_BOOT_UART_DM_CR(base));
  writel(MSM_BOOT_UART_DM_CMD_RESET_TX, MSM_BOOT_UART_DM_CR(base));
  writel(MSM_BOOT_UART_DM_CMD_RESET_ERR_STAT, MSM_BOOT_UART_DM_CR(base));
  writel(MSM_BOOT_UART_DM_CMD_RES_TX_ERR, MSM_BOOT_UART_DM_CR(base));
  writel(MSM_BOOT_UART_DM_CMD_RES_STALE_INT, MSM_BOOT_UART_DM_CR(base));

  return MSM_BOOT_UART_DM_E_SUCCESS;
}

/*
 * Initialize UART_DM - configure clock and required registers.
 */
static unsigned int msm_boot_uart_dm_init(uint32_t uart_dm_base)
{
  /* Configure UART mode registers MR1 and MR2 */
  /* Hardware flow control isn't supported */
  writel(0x0, MSM_BOOT_UART_DM_MR1(uart_dm_base));

  /* 8-N-1 configuration: 8 data bits - No parity - 1 stop bit */
  writel(MSM_BOOT_UART_DM_8_N_1_MODE, MSM_BOOT_UART_DM_MR2(uart_dm_base));

  /* Configure Interrupt Mask register IMR */
  writel(MSM_BOOT_UART_DM_IMR_ENABLED, MSM_BOOT_UART_DM_IMR(uart_dm_base));

  /* Configure Tx and Rx watermarks configuration registers */
  /* TX watermark value is set to 0 - interrupt is generated when
   * FIFO level is less than or equal to 0 */
  writel(MSM_BOOT_UART_DM_TFW_VALUE, MSM_BOOT_UART_DM_TFWR(uart_dm_base));

  /* RX watermark value */
  writel(MSM_BOOT_UART_DM_RFW_VALUE, MSM_BOOT_UART_DM_RFWR(uart_dm_base));

  /* Configure Interrupt Programming Register */
  /* Set initial Stale timeout value */
  writel(
      MSM_BOOT_UART_DM_STALE_TIMEOUT_LSB, MSM_BOOT_UART_DM_IPR(uart_dm_base));

  /* Configure IRDA if required */
  /* Disabling IRDA mode */
  writel(0x0, MSM_BOOT_UART_DM_IRDA(uart_dm_base));

  /* Configure and enable sim interface if required */

  /* Configure hunt character value in HCR register */
  /* Keep it in reset state */
  writel(0x0, MSM_BOOT_UART_DM_HCR(uart_dm_base));

  /* Configure Rx FIFO base address */
  /* Both TX/RX shares same SRAM and default is half-n-half.
   * Sticking with default value now.
   * As such RAM size is (2^RAM_ADDR_WIDTH, 32-bit entries).
   * We have found RAM_ADDR_WIDTH = 0x7f */

  /* Issue soft reset command */
  msm_boot_uart_dm_reset(uart_dm_base);

  /* Enable/Disable Rx/Tx DM interfaces */
  /* Data Mover not currently utilized. */
  writel(0x0, MSM_BOOT_UART_DM_DMEN(uart_dm_base));

  /* Enable transmitter and receiver */
  writel(MSM_BOOT_UART_DM_CR_RX_ENABLE, MSM_BOOT_UART_DM_CR(uart_dm_base));
  writel(MSM_BOOT_UART_DM_CR_TX_ENABLE, MSM_BOOT_UART_DM_CR(uart_dm_base));

  /* Initialize Receive Path */
  msm_boot_uart_dm_init_rx_transfer(uart_dm_base);

  return MSM_BOOT_UART_DM_E_SUCCESS;
}

/*
 * Initialize Receive Path
 */
static unsigned int msm_boot_uart_dm_init_rx_transfer(uint32_t uart_dm_base)
{
  writel(
      MSM_BOOT_UART_DM_GCMD_DIS_STALE_EVT, MSM_BOOT_UART_DM_CR(uart_dm_base));
  writel(MSM_BOOT_UART_DM_CMD_RES_STALE_INT, MSM_BOOT_UART_DM_CR(uart_dm_base));
  writel(MSM_BOOT_UART_DM_DMRX_DEF_VALUE, MSM_BOOT_UART_DM_DMRX(uart_dm_base));
  writel(
      MSM_BOOT_UART_DM_GCMD_ENA_STALE_EVT, MSM_BOOT_UART_DM_CR(uart_dm_base));

  return MSM_BOOT_UART_DM_E_SUCCESS;
}

/* Defining functions that's exposed to outside world and in coformance to
 * existing uart implemention. These functions are being called to initialize
 * UART and print debug messages in bootloader.
 */
void uart_dm_init(uint8_t id, uint32_t gsbi_base, uint32_t uart_dm_base)
{
  /* Configure the uart clock */
  LibQcomPlatformUartDmClockConfig(id);
  dsb();

  /* Configure GPIO to provide connectivity between UART block
     product ports and chip pads */
  LibQcomPlatformUartDmGpioConfig(id);
  dsb();

  /* Configure GSBI for UART_DM protocol.
   * I2C on 2 ports, UART (without HS flow control) on the other 2.
   * This is only on chips that have GSBI block
   */
  if (gsbi_base)
    writel(
        GSBI_PROTOCOL_CODE_I2C_UART << GSBI_CTRL_REG_PROTOCOL_CODE_S,
        GSBI_CTRL_REG(gsbi_base));
  dsb();

  /* Configure clock selection register for tx and rx rates.
   * Selecting 115.2k for both RX and TX.
   */
  writel(
      (UINTN)PcdGet64(PcdUartDmClkRxTxBitRate),
      MSM_BOOT_UART_DM_CSR(uart_dm_base));
  dsb();

  /* Intialize UART_DM */
  msm_boot_uart_dm_init(uart_dm_base);
}
