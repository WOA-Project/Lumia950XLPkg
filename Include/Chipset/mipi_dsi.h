/* Copyright (c) 2010-2015, The Linux Foundation. All rights reserved.
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
 *
 */

#ifndef _PLATFORM_MSM_SHARED_MIPI_DSI_H_
#define _PLATFORM_MSM_SHARED_MIPI_DSI_H_

#include <Chipset/msm_panel.h>

#define PASS 0
#define FAIL 1

#define DSI_VIDEO_MODE_DONE_MASK BIT(17)
#define DSI_VIDEO_MODE_DONE_AK BIT(16)
#define DSI_VIDEO_MODE_DONE_STAT BIT(16)

/**********************************************************
  DSI register configuration options
 **********************************************************/
#define MIPI_DSI_MRPS 0x04  /* Maximum Return Packet Size */
#define MIPI_DSI_REG_LEN 16 /* 4 x 4 bytes register */

#define TIMING_FLUSH 0x1E4
#define TIMING_DB_MODE 0x1E8

#define DSI_HW_REV_103 0x10030000   /* 8994 */
#define DSI_HW_REV_103_1 0x10030001 /* 8936/8939 */

#define DTYPE_GEN_WRITE2 0x23 /* 4th Byte is 0x80 */
#define DTYPE_GEN_LWRITE 0x29 /* 4th Byte is 0xc0 */
#define DTYPE_DCS_WRITE1 0x15 /* 4th Byte is 0x80 */

#define RDBK_DATA0 0x06C

#define MIPI_VIDEO_MODE 1
#define MIPI_CMD_MODE 2

#define DSI_NON_BURST_SYNCH_PULSE 0
#define DSI_NON_BURST_SYNCH_EVENT 1
#define DSI_BURST_MODE 2

#define DSI_RGB_SWAP_RGB 0
#define DSI_RGB_SWAP_RBG 1
#define DSI_RGB_SWAP_BGR 2
#define DSI_RGB_SWAP_BRG 3
#define DSI_RGB_SWAP_GRB 4
#define DSI_RGB_SWAP_GBR 5

#define DSI_VIDEO_DST_FORMAT_RGB565 0
#define DSI_VIDEO_DST_FORMAT_RGB666 1
#define DSI_VIDEO_DST_FORMAT_RGB666_LOOSE 2
#define DSI_VIDEO_DST_FORMAT_RGB888 3

#define DSI_CMD_DST_FORMAT_RGB111 0
#define DSI_CMD_DST_FORMAT_RGB332 3
#define DSI_CMD_DST_FORMAT_RGB444 4
#define DSI_CMD_DST_FORMAT_RGB565 6
#define DSI_CMD_DST_FORMAT_RGB666 7
#define DSI_CMD_DST_FORMAT_RGB888 8

#define DSI_CMD_TRIGGER_NONE 0x0 /* mdp trigger */
#define DSI_CMD_TRIGGER_TE 0x02
#define DSI_CMD_TRIGGER_SW 0x04
#define DSI_CMD_TRIGGER_SW_SEOF 0x05 /* cmd dma only */
#define DSI_CMD_TRIGGER_SW_TE 0x06

#define DSI_DATALANE_SWAP_0123 0
#define DSI_DATALANE_SWAP_3012 1
#define DSI_DATALANE_SWAP_2301 2
#define DSI_DATALANE_SWAP_1230 3
#define DSI_DATALANE_SWAP_0321 4
#define DSI_DATALANE_SWAP_1032 5
#define DSI_DATALANE_SWAP_2103 6
#define DSI_DATALANE_SWAP_3210 7

#define MAX_REGULATOR_CONFIG 7
#define MAX_BIST_CONFIG 6
#define MAX_TIMING_CONFIG 12
#define MAX_LANE_CONFIG 45
#define MAX_STRENGTH_CONFIG 2
#define MAX_CTRL_CONFIG 4

/**********************************************************
  DSI configuration structures
 **********************************************************/
struct mipi_dsi_phy_ctrl {
  uint32_t regulator[5];
  uint32_t timing[12];
  uint32_t ctrl[4];
  uint32_t strength[4];
  uint32_t pll[21];
};

enum dsi_reg_mode {
  DSI_PHY_REGULATOR_DCDC_MODE,
  DSI_PHY_REGULATOR_LDO_MODE,
};

struct mdss_dsi_phy_ctrl {
  uint32_t          regulator[MAX_REGULATOR_CONFIG];
  uint32_t          timing[MAX_TIMING_CONFIG];
  uint32_t          ctrl[MAX_CTRL_CONFIG];
  uint32_t          strength[MAX_STRENGTH_CONFIG];
  char              bistCtrl[MAX_BIST_CONFIG];
  char              laneCfg[MAX_LANE_CONFIG];
  enum dsi_reg_mode regulator_mode;
  int               is_pll_20nm;
};

struct mdss_dsi_pll_config {
  uint32_t pixel_clock;
  uint32_t pixel_clock_mhz;
  uint32_t byte_clock;
  uint32_t bit_clock;
  uint32_t halfbit_clock;
  uint32_t vco_clock;
  uint32_t vco_delay;
  uint32_t vco_min;
  uint32_t vco_max;
  uint32_t en_vco_zero_phase;
  uint8_t  directpath;
  uint8_t  posdiv1;
  uint8_t  posdiv3;
  uint8_t  pclk_m;
  uint8_t  pclk_n;
  uint8_t  pclk_d;

  /* pll 20nm */
  uint32_t dec_start;
  uint32_t frac_start;
  uint32_t lock_comp;
  uint8_t  hr_oclk2;
  uint8_t  hr_oclk3;
  uint8_t  lp_div_mux;
  uint8_t  ndiv;
};

struct mipi_dsi_cmd {
  uint32_t size;
  char *   payload;
  int      wait;
};

struct mipi_dsi_panel_config {
  char                      mode;
  char                      num_of_lanes;
  char                      lane_swap;
  char                      pack;
  uint8_t                   t_clk_pre;
  uint8_t                   t_clk_post;
  struct mipi_dsi_phy_ctrl *dsi_phy_config;
  struct mdss_dsi_phy_ctrl *mdss_dsi_phy_config;
  struct mipi_dsi_cmd *     panel_on_cmds;
  int                       num_of_panel_on_cmds;
  uint32_t                  signature;
  char                      cmds_post_tg;
};

enum { /* mipi dsi panel */
       DSI_VIDEO_MODE,
       DSI_CMD_MODE,
};

/**********************************************************
  APIs
 **********************************************************/
int  mdss_dsi_config(struct msm_fb_panel_data *panel);
void mdss_dsi_phy_sw_reset(uint32_t ctl_base);
int  mdss_dsi_phy_init(struct mipi_panel_info *mipi);
void mdss_dsi_phy_contention_detection(
    struct mipi_panel_info *mipi, uint32_t phy_base);

int mdss_dsi_video_mode_config(
    uint16_t disp_width, uint16_t disp_height, uint16_t img_width,
    uint16_t img_height, uint16_t hsync_porch0_fp, uint16_t hsync_porch0_bp,
    uint16_t vsync_porch0_fp, uint16_t vsync_porch0_bp, uint16_t hsync_width,
    uint16_t vsync_width, uint16_t dst_format, uint16_t traffic_mode,
    uint8_t lane_en, uint8_t pulse_mode_hsa_he, uint32_t low_pwr_stop_mode,
    uint8_t eof_bllp_pwr, uint8_t interleav, uint32_t ctl_base);
int mdss_dsi_cmd_mode_config(
    uint16_t disp_width, uint16_t disp_height, uint16_t img_width,
    uint16_t img_height, uint16_t dst_format, uint8_t ystride, uint8_t lane_en,
    uint8_t interleav, uint32_t ctl_base);

int mipi_dsi_on(struct msm_panel_info *pinfo);
int mipi_dsi_off(struct msm_panel_info *pinfo);
int mdss_dsi_cmds_tx(
    struct mipi_panel_info *mipi, struct mipi_dsi_cmd *cmds, int count,
    char dual_dsi);
int mdss_dsi_cmds_rx(
    struct mipi_panel_info *mipi, uint32_t **rp, int rp_len, int rdbk_len);
int32_t mdss_dsi_auto_pll_config(
    uint32_t pll_base, uint32_t ctl_base, struct mdss_dsi_pll_config *pd);
void     mdss_dsi_auto_pll_20nm_config(struct msm_panel_info *pinfo);
void     mdss_dsi_pll_20nm_sw_reset_st_machine(uint32_t pll_base);
uint32_t mdss_dsi_pll_20nm_lock_status(uint32_t pll_base);
void     mdss_dsi_uniphy_pll_lock_detect_setting(uint32_t pll_base);
void     mdss_dsi_uniphy_pll_sw_reset(uint32_t pll_base);
int      mdss_dsi_post_on(struct msm_fb_panel_data *panel);
#endif
