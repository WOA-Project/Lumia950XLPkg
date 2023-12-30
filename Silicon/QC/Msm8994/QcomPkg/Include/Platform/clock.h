/* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
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

#ifndef __MSM8994_CLOCK_H
#define __MSM8994_CLOCK_H

#include <Chipset/clock.h>
#include <Chipset/clock_lib2.h>

#define UART_DM_CLK_RX_TX_BIT_RATE 0xCC

#define REG_MM(off) (MSM_MMSS_CLK_CTL_BASE + (off))

#define MDP_GDSCR REG_MM(0x2304)
#define GDSC_POWER_ON_BIT BIT(31)
#define GDSC_POWER_ON_STATUS_BIT BIT(29)
#define GDSC_EN_FEW_WAIT_MASK (0x0F << 16)
#define GDSC_EN_FEW_WAIT_256_MASK BIT(19)

#define VSYNC_CMD_RCGR REG_MM(0x2080)
#define VSYNC_CFG_RCGR REG_MM(0x2084)
#define MDSS_VSYNC_CBCR REG_MM(0x2328)
#define MDP_CMD_RCGR REG_MM(0x2040)
#define MDP_CFG_RCGR REG_MM(0x2044)
#define MDP_CBCR REG_MM(0x231C)
#define MDP_LUT_CBCR REG_MM(0x2320)
#define MDP_AHB_CBCR REG_MM(0x2308)
#define MDSS_HDMI_AHB_CBCR REG_MM(0x230C)
#define MDSS_HDMI_CBCR REG_MM(0x2338)
#define MDSS_EXTPCLK_CBCR REG_MM(0x2324)
#define EXTPCLK_CMD_RCGR REG_MM(0x2060)
#define EXTPCLK_CFG_RCGR REG_MM(0x2064)
#define HDMI_CMD_RCGR REG_MM(0x2100)
#define HDMI_CFG_RCGR REG_MM(0x2104)

#define MDP_AXI_CMD_RCGR REG_MM(0x5040)
#define MDP_AXI_CFG_RCGR REG_MM(0x5044)

#define MDP_AXI_CBCR REG_MM(0x2310)
#define MMSS_S0_AXI_CBCR REG_MM(0x5064)
#define MMSS_MMSSNOC_AXI_CBCR REG_MM(0x506C)

#define DSI_BYTE0_CMD_RCGR REG_MM(0x2120)
#define DSI_BYTE0_CFG_RCGR REG_MM(0x2124)
#define DSI_BYTE0_CBCR REG_MM(0x233C)
#define DSI_ESC0_CMD_RCGR REG_MM(0x2160)
#define DSI_ESC0_CFG_RCGR REG_MM(0x2164)
#define DSI_ESC0_CBCR REG_MM(0x2344)
#define DSI_PIXEL0_CMD_RCGR REG_MM(0x2000)
#define DSI_PIXEL0_CFG_RCGR REG_MM(0x2004)
#define DSI_PIXEL0_CBCR REG_MM(0x2314)
#define DSI_PIXEL0_M REG_MM(0x2008)
#define DSI_PIXEL0_N REG_MM(0x200C)
#define DSI_PIXEL0_D REG_MM(0x2010)

#define DSI0_PHY_PLL_OUT BIT(8)
#define PIXEL_SRC_DIV_1_5 BIT(1)

#define DSI_BYTE1_CMD_RCGR REG_MM(0x2140)
#define DSI_BYTE1_CFG_RCGR REG_MM(0x2144)
#define DSI_BYTE1_CBCR REG_MM(0x2340)
#define DSI_ESC1_CMD_RCGR REG_MM(0x2180)
#define DSI_ESC1_CFG_RCGR REG_MM(0x2184)
#define DSI_ESC1_CBCR REG_MM(0x2348)
#define DSI_PIXEL1_CMD_RCGR REG_MM(0x2020)
#define DSI_PIXEL1_CFG_RCGR REG_MM(0x2024)
#define DSI_PIXEL1_CBCR REG_MM(0x2318)
#define DSI_PIXEL1_M REG_MM(0x2028)
#define DSI_PIXEL1_N REG_MM(0x202C)
#define DSI_PIXEL1_D REG_MM(0x2030)

#define MDSS_EDPPIXEL_CBCR REG_MM(0x232C)
#define MDSS_EDPLINK_CBCR REG_MM(0x2330)
#define MDSS_EDPAUX_CBCR REG_MM(0x2334)
#define EDPPIXEL_M REG_MM(0x20A8)
#define EDPPIXEL_N REG_MM(0x20AC)
#define EDPPIXEL_D REG_MM(0x20B0)
#define EDPPIXEL_CFG_RCGR REG_MM(0x20A4)
#define EDPPIXEL_CMD_RCGR REG_MM(0x20A0)
#define EDPLINK_CFG_RCGR REG_MM(0x20C4)
#define EDPLINK_CMD_RCGR REG_MM(0x20C0)
#define EDPAUX_CFG_RCGR REG_MM(0x20E4)
#define EDPAUX_CMD_RCGR REG_MM(0x20E0)

#define MMSS_DSI_CLKS_FLAG_DSI0 BIT(0)
#define MMSS_DSI_CLKS_FLAG_DSI1 BIT(1)

void platform_clock_init(void);

void clock_init_mmc(uint32_t interface);
void clock_config_mmc(uint32_t interface, uint32_t freq);
void clock_config_uart_dm(uint8_t id);
void hsusb_clock_init(void);
void clock_config_ce(uint8_t instance);
void mdp_clock_init(void);
void clock_ce_enable(uint8_t instance);
void clock_ce_disable(uint8_t instance);
void clock_usb30_init(void);

void mmss_dsi_clock_enable(
    uint32_t dsi_pixel0_cfg_rcgr, uint32_t dual_dsi, uint8_t pclk0_m,
    uint8_t pclk0_n, uint8_t pclk0_d);
void mmss_dsi_clock_disable(uint32_t dual_dsi);
void mmss_bus_clock_enable(void);
void mmss_bus_clock_disable(void);
void mdp_clock_enable(void);
void mdp_clock_disable(void);
void hdmi_core_ahb_clk_enable(void);
void hdmi_pixel_clk_enable(uint32_t rate);
void hdmi_pixel_clk_disable(void);
void hdmi_core_ahb_clk_disable(void);

#endif
