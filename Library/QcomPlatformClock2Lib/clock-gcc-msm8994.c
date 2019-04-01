#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <GplLibrary/clock-gcc-msm8994.h>

#define gcc_xo_source_val 0
#define gpll0_out_main_source_val 1
#define gpll4_out_main_source_val 5
#define pcie_pipe_source_val 2

#define BM(msb, lsb)                                                           \
  (((((uint32_t)-1) << (31 - msb)) >> (31 - msb + lsb)) << lsb)
#define BVAL(msb, lsb, val) (((val) << lsb) & BM(msb, lsb))
#define FIXDIV(div) (div ? (2 * (div)-1) : (0))

#define GPLL0_MODE (0x0000)
#define SYS_NOC_USB3_AXI_CBCR (0x03FC)
#define SYS_NOC_UFS_AXI_CBCR (0x1D7C)
#define MSS_CFG_AHB_CBCR (0x0280)
#define MSS_Q6_BIMC_AXI_CBCR (0x0284)
#define USB_30_BCR (0x03C0)
#define USB30_MASTER_CBCR (0x03C8)
#define USB30_SLEEP_CBCR (0x03CC)
#define USB30_MOCK_UTMI_CBCR (0x03D0)
#define USB30_MASTER_CMD_RCGR (0x03D4)
#define USB30_MOCK_UTMI_CMD_RCGR (0x03E8)
#define USB3_PHY_BCR (0x1400)
#define USB3PHY_PHY_BCR (0x1404)
#define USB3_PHY_AUX_CBCR (0x1408)
#define USB3_PHY_PIPE_CBCR (0x140C)
#define USB3_PHY_AUX_CMD_RCGR (0x1414)
#define USB_HS_BCR (0x0480)
#define USB_HS_SYSTEM_CBCR (0x0484)
#define USB_HS_AHB_CBCR (0x0488)
#define USB_HS_SYSTEM_CMD_RCGR (0x0490)
#define USB2_HS_PHY_SLEEP_CBCR (0x04AC)
#define USB2_HS_PHY_ONLY_BCR (0x04B0)
#define QUSB2_PHY_BCR (0x04B8)
#define USB_PHY_CFG_AHB2PHY_CBCR (0x1A84)
#define SDCC1_APPS_CMD_RCGR (0x04D0)
#define SDCC1_APPS_CBCR (0x04C4)
#define SDCC1_AHB_CBCR (0x04C8)
#define SDCC2_APPS_CMD_RCGR (0x0510)
#define SDCC2_APPS_CBCR (0x0504)
#define SDCC2_AHB_CBCR (0x0508)
#define SDCC3_APPS_CMD_RCGR (0x0550)
#define SDCC3_APPS_CBCR (0x0544)
#define SDCC3_AHB_CBCR (0x0548)
#define SDCC4_APPS_CMD_RCGR (0x0590)
#define SDCC4_APPS_CBCR (0x0584)
#define SDCC4_AHB_CBCR (0x0588)
#define BLSP1_AHB_CBCR (0x05C4)
#define BLSP1_QUP1_SPI_APPS_CBCR (0x0644)
#define BLSP1_QUP1_I2C_APPS_CBCR (0x0648)
#define BLSP1_QUP1_I2C_APPS_CMD_RCGR (0x0660)
#define BLSP1_QUP2_I2C_APPS_CMD_RCGR (0x06E0)
#define BLSP1_QUP3_I2C_APPS_CMD_RCGR (0x0760)
#define BLSP1_QUP4_I2C_APPS_CMD_RCGR (0x07E0)
#define BLSP1_QUP5_I2C_APPS_CMD_RCGR (0x0860)
#define BLSP1_QUP6_I2C_APPS_CMD_RCGR (0x08E0)
#define BLSP2_QUP1_I2C_APPS_CMD_RCGR (0x09A0)
#define BLSP2_QUP2_I2C_APPS_CMD_RCGR (0x0A20)
#define BLSP2_QUP3_I2C_APPS_CMD_RCGR (0x0AA0)
#define BLSP2_QUP4_I2C_APPS_CMD_RCGR (0x0B20)
#define BLSP2_QUP5_I2C_APPS_CMD_RCGR (0x0BA0)
#define BLSP2_QUP6_I2C_APPS_CMD_RCGR (0x0C20)
#define BLSP1_QUP1_SPI_APPS_CMD_RCGR (0x064C)
#define BLSP1_UART1_APPS_CBCR (0x0684)
#define BLSP1_UART1_APPS_CMD_RCGR (0x068C)
#define BLSP1_QUP2_SPI_APPS_CBCR (0x06C4)
#define BLSP1_QUP2_I2C_APPS_CBCR (0x06C8)
#define BLSP1_QUP2_SPI_APPS_CMD_RCGR (0x06CC)
#define BLSP1_UART2_APPS_CBCR (0x0704)
#define BLSP1_UART2_APPS_CMD_RCGR (0x070C)
#define BLSP1_QUP3_SPI_APPS_CBCR (0x0744)
#define BLSP1_QUP3_I2C_APPS_CBCR (0x0748)
#define BLSP1_QUP3_SPI_APPS_CMD_RCGR (0x074C)
#define BLSP1_UART3_APPS_CBCR (0x0784)
#define BLSP1_UART3_APPS_CMD_RCGR (0x078C)
#define BLSP1_QUP4_SPI_APPS_CBCR (0x07C4)
#define BLSP1_QUP4_I2C_APPS_CBCR (0x07C8)
#define BLSP1_QUP4_SPI_APPS_CMD_RCGR (0x07CC)
#define BLSP1_UART4_APPS_CBCR (0x0804)
#define BLSP1_UART4_APPS_CMD_RCGR (0x080C)
#define BLSP1_QUP5_SPI_APPS_CBCR (0x0844)
#define BLSP1_QUP5_I2C_APPS_CBCR (0x0848)
#define BLSP1_QUP5_SPI_APPS_CMD_RCGR (0x084C)
#define BLSP1_UART5_APPS_CBCR (0x0884)
#define BLSP1_UART5_APPS_CMD_RCGR (0x088C)
#define BLSP1_QUP6_SPI_APPS_CBCR (0x08C4)
#define BLSP1_QUP6_I2C_APPS_CBCR (0x08C8)
#define BLSP1_QUP6_SPI_APPS_CMD_RCGR (0x08CC)
#define BLSP1_UART6_APPS_CBCR (0x0904)
#define BLSP1_UART6_APPS_CMD_RCGR (0x090C)
#define BLSP2_AHB_CBCR (0x0944)
#define BLSP2_QUP1_SPI_APPS_CBCR (0x0984)
#define BLSP2_QUP1_I2C_APPS_CBCR (0x0988)
#define BLSP2_QUP1_SPI_APPS_CMD_RCGR (0x098C)
#define BLSP2_UART1_APPS_CBCR (0x09C4)
#define BLSP2_UART1_APPS_CMD_RCGR (0x09CC)
#define BLSP2_QUP2_SPI_APPS_CBCR (0x0A04)
#define BLSP2_QUP2_I2C_APPS_CBCR (0x0A08)
#define BLSP2_QUP2_SPI_APPS_CMD_RCGR (0x0A0C)
#define BLSP2_UART2_APPS_CBCR (0x0A44)
#define BLSP2_UART2_APPS_CMD_RCGR (0x0A4C)
#define BLSP2_QUP3_SPI_APPS_CBCR (0x0A84)
#define BLSP2_QUP3_I2C_APPS_CBCR (0x0A88)
#define BLSP2_QUP3_SPI_APPS_CMD_RCGR (0x0A8C)
#define BLSP2_UART3_APPS_CBCR (0x0AC4)
#define BLSP2_UART3_APPS_CMD_RCGR (0x0ACC)
#define BLSP2_QUP4_SPI_APPS_CBCR (0x0B04)
#define BLSP2_QUP4_I2C_APPS_CBCR (0x0B08)
#define BLSP2_QUP4_SPI_APPS_CMD_RCGR (0x0B0C)
#define BLSP2_UART4_APPS_CBCR (0x0B44)
#define BLSP2_UART4_APPS_CMD_RCGR (0x0B4C)
#define BLSP2_QUP5_SPI_APPS_CBCR (0x0B84)
#define BLSP2_QUP5_I2C_APPS_CBCR (0x0B88)
#define BLSP2_QUP5_SPI_APPS_CMD_RCGR (0x0B8C)
#define BLSP2_UART5_APPS_CBCR (0x0BC4)
#define BLSP2_UART5_APPS_CMD_RCGR (0x0BCC)
#define BLSP2_QUP6_SPI_APPS_CBCR (0x0C04)
#define BLSP2_QUP6_I2C_APPS_CBCR (0x0C08)
#define BLSP2_QUP6_SPI_APPS_CMD_RCGR (0x0C0C)
#define BLSP2_UART6_APPS_CBCR (0x0C44)
#define BLSP2_UART6_APPS_CMD_RCGR (0x0C4C)
#define PDM_AHB_CBCR (0x0CC4)
#define PDM2_CBCR (0x0CCC)
#define PDM2_CMD_RCGR (0x0CD0)
#define PRNG_AHB_CBCR (0x0D04)
#define BAM_DMA_AHB_CBCR (0x0D44)
#define TSIF_AHB_CBCR (0x0D84)
#define TSIF_REF_CBCR (0x0D88)
#define TSIF_REF_CMD_RCGR (0x0D90)
#define BOOT_ROM_AHB_CBCR (0x0E04)
#define GCC_XO_DIV4_CBCR (0x10C8)
#define LPASS_Q6_AXI_CBCR (0x11C0)
#define APCS_GPLL_ENA_VOTE (0x1480)
#define APCS_CLOCK_BRANCH_ENA_VOTE (0x1484)
#define GCC_DEBUG_CLK_CTL (0x1880)
#define CLOCK_FRQ_MEASURE_CTL (0x1884)
#define CLOCK_FRQ_MEASURE_STATUS (0x1888)
#define PLLTEST_PAD_CFG (0x188C)
#define GP1_CBCR (0x1900)
#define GP1_CMD_RCGR (0x1904)
#define GP2_CBCR (0x1940)
#define GP2_CMD_RCGR (0x1944)
#define GP3_CBCR (0x1980)
#define GP3_CMD_RCGR (0x1984)
#define GPLL4_MODE (0x1DC0)
#define PCIE_0_SLV_AXI_CBCR (0x1AC8)
#define PCIE_0_MSTR_AXI_CBCR (0x1ACC)
#define PCIE_0_CFG_AHB_CBCR (0x1AD0)
#define PCIE_0_AUX_CBCR (0x1AD4)
#define PCIE_0_PIPE_CBCR (0x1AD8)
#define PCIE_0_PIPE_CMD_RCGR (0x1ADC)
#define PCIE_0_AUX_CMD_RCGR (0x1B00)
#define PCIE_PHY_0_PHY_BCR (0x1B14)
#define PCIE_PHY_0_BCR (0x1B18)
#define PCIE_1_SLV_AXI_CBCR (0x1B48)
#define PCIE_1_MSTR_AXI_CBCR (0x1B4C)
#define PCIE_1_CFG_AHB_CBCR (0x1B50)
#define PCIE_1_AUX_CBCR (0x1B54)
#define PCIE_1_PIPE_CBCR (0x1B58)
#define PCIE_1_PIPE_CMD_RCGR (0x1B5C)
#define PCIE_1_AUX_CMD_RCGR (0x1B80)
#define PCIE_PHY_1_PHY_BCR (0x1B94)
#define PCIE_PHY_1_BCR (0x1B98)
#define UFS_AXI_CBCR (0x1D48)
#define UFS_AHB_CBCR (0x1D4C)
#define UFS_TX_CFG_CBCR (0x1D50)
#define UFS_RX_CFG_CBCR (0x1D54)
#define UFS_TX_SYMBOL_0_CBCR (0x1D58)
#define UFS_TX_SYMBOL_1_CBCR (0x1D5C)
#define UFS_RX_SYMBOL_0_CBCR (0x1D60)
#define UFS_RX_SYMBOL_1_CBCR (0x1D64)
#define UFS_AXI_CMD_RCGR (0x1D68)
#define PCIE_0_PHY_LDO_EN (0x1E00)
#define PCIE_1_PHY_LDO_EN (0x1E04)
#define USB_SS_PHY_LDO_EN (0x1E08)
#define UFS_PHY_LDO_EN (0x1E0C)
#define AXI_CMD_RCGR (0x5040)

#define MSM8994_GCC_BASE 0xfc400000

#define CMD_RCGR_REG(x) (VOID *)((UINTN)MSM8994_GCC_BASE + x)
#define CFG_RCGR_REG(x) (VOID *)((UINTN)MSM8994_GCC_BASE + x + 0x4)
#define M_REG(x) (VOID *)((UINTN)MSM8994_GCC_BASE + x + 0x8)
#define N_REG(x) (VOID *)((UINTN)MSM8994_GCC_BASE + x + 0xC)
#define D_REG(x) (VOID *)((UINTN)MSM8994_GCC_BASE + x + 0x10)
#define CBCR_REG(x) (VOID *)((UINTN)MSM8994_GCC_BASE + x)
#define BCR_REG(x) (VOID *)((UINTN)MSM8994_GCC_BASE + x)
#define RST_REG(x) (VOID *)((UINTN)MSM8994_GCC_BASE + x)
#define VOTE_REG(x) (VOID *)((UINTN)MSM8994_GCC_BASE + x)
#define GATE_EN_REG(x) (VOID *)((UINTN)MSM8994_GCC_BASE + x)

/*
 * Important clock bit positions and masks
 */
#define CMD_RCGR_ROOT_ENABLE_BIT BIT(1)
#define CBCR_BRANCH_ENABLE_BIT BIT(0)
#define CBCR_BRANCH_OFF_BIT BIT(31)
#define CMD_RCGR_CONFIG_UPDATE_BIT BIT(0)
#define CMD_RCGR_ROOT_STATUS_BIT BIT(31)
#define BCR_BLK_ARES_BIT BIT(0)
#define CBCR_HW_CTL_BIT BIT(1)
#define CFG_RCGR_DIV_MASK BM(4, 0)
#define CFG_RCGR_SRC_SEL_MASK BM(10, 8)
#define MND_MODE_MASK BM(13, 12)
#define MND_DUAL_EDGE_MODE_BVAL BVAL(13, 12, 0x2)
#define CMD_RCGR_CONFIG_DIRTY_MASK BM(7, 4)
#define CBCR_CDIV_LSB 16
#define CBCR_CDIV_MSB 19

/*
 * When updating an RCG configuration, check the update bit up to this number
 * number of times (with a 1 us delay in between) before continuing.
 */
#define UPDATE_CHECK_MAX_LOOPS 500

/*
 * When enabling/disabling a clock, check the halt bit up to this number
 * number of times (with a 1 us delay in between) before continuing.
 */
#define HALT_CHECK_MAX_LOOPS 500
/* For clock without halt checking, wait this long after enables/disables. */
#define HALT_CHECK_DELAY_US 500

/*
 * Halt/Status Checking Mode Macros
 */
#define HALT 0         /* Bit pol: 1 = halted */
#define NOCHECK 1      /* No bit to check, do nothing */
#define HALT_VOTED 2   /* Bit pol: 1 = halted; delay on disable */
#define ENABLE 3       /* Bit pol: 1 = running */
#define ENABLE_VOTED 4 /* Bit pol: 1 = running; delay on disable */
#define DELAY 5        /* No bit to check, just delay */

enum branch_state {
  BRANCH_ON,
  BRANCH_OFF,
};

/* I know the naming is cursed, but Qualcomm code is even more cursed
 * Baseline reference:
 *
   static struct clk_freq_tbl ftbl_pcie_pipe_clk_src[] = {
        F_EXT( 125000000,      pcie_pipe,    1,    0,     0),
        F_END
   };
 *
 * static struct rcg_clk pcie_1_pipe_clk_src = {
        .cmd_rcgr_reg = PCIE_1_PIPE_CMD_RCGR,
        .set_rate = set_rate_hid,
        .freq_tbl = ftbl_pcie_pipe_clk_src,
        .current_freq = &rcg_dummy_freq,
        .base = &virt_base,
        .c = {
                .dbg_name = "pcie_1_pipe_clk_src",
                .ops = &clk_ops_rcg,
                VDD_DIG_FMAX_MAP2(LOWER, 62500000, LOW, 125000000),
                CLK_INIT(pcie_1_pipe_clk_src.c),
        },
 };
 */
VOID EFIAPI rcg_update_config(UINTN rcg)
{
  u32 cmd_rcgr_regval, count;

  cmd_rcgr_regval = readl_relaxed(CMD_RCGR_REG(rcg));
  cmd_rcgr_regval |= CMD_RCGR_CONFIG_UPDATE_BIT;
  writel_relaxed(cmd_rcgr_regval, CMD_RCGR_REG(rcg));

  for (count = UPDATE_CHECK_MAX_LOOPS; count > 0; count--) {
    if (!(readl_relaxed(CMD_RCGR_REG(rcg)) & CMD_RCGR_CONFIG_UPDATE_BIT))
      return;
    udelay(1);
  }

  DEBUG((EFI_D_ERROR, "RCG didn't update config \n"));
  CpuDeadLoop();
}

VOID EFIAPI pcie_0_pipe_clk_set_rate_enable(VOID)
{
  EFI_TPL OldTpl;
  u32     cfg_regval;

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Parent clock: __clk_pre_reparent, ignore

  // 2. Set rate
  cfg_regval = readl_relaxed(CFG_RCGR_REG(PCIE_0_PIPE_CMD_RCGR));
  cfg_regval &= ~(CFG_RCGR_DIV_MASK | CFG_RCGR_SRC_SEL_MASK);
  cfg_regval |= BVAL(4, 0, (int)FIXDIV(1)) | BVAL(10, 8, pcie_pipe_source_val);
  writel_relaxed(cfg_regval, CFG_RCGR_REG(PCIE_0_PIPE_CMD_RCGR));

  // 3. Update RCG config
  rcg_update_config(PCIE_0_PIPE_CMD_RCGR);

  // 4. TPL
  gBS->RestoreTPL(OldTpl);
}

VOID EFIAPI pcie_1_pipe_clk_set_rate_enable(VOID)
{
  EFI_TPL OldTpl;
  u32     cfg_regval;

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Parent clock: __clk_pre_reparent, ignore

  // 2. Set rate
  cfg_regval = readl_relaxed(CFG_RCGR_REG(PCIE_1_PIPE_CMD_RCGR));
  cfg_regval &= ~(CFG_RCGR_DIV_MASK | CFG_RCGR_SRC_SEL_MASK);
  cfg_regval |= BVAL(4, 0, (int)FIXDIV(1)) | BVAL(10, 8, pcie_pipe_source_val);
  writel_relaxed(cfg_regval, CFG_RCGR_REG(PCIE_1_PIPE_CMD_RCGR));

  // 3. Update RCG config
  rcg_update_config(PCIE_1_PIPE_CMD_RCGR);

  // 4. TPL
  gBS->RestoreTPL(OldTpl);
}

VOID EFIAPI pcie_0_aux_clk_set_rate_and_enable(VOID)
{
  EFI_TPL OldTpl;
  u32     cfg_regval;
  u32     n_val = ~(19 - 1) * !!(19);

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Parent clock: __clk_pre_reparent, ignore

  // 2. Set rate
  cfg_regval = readl_relaxed(CFG_RCGR_REG(PCIE_0_AUX_CMD_RCGR));
  writel_relaxed(1, M_REG(PCIE_0_AUX_CMD_RCGR));
  writel_relaxed(n_val, N_REG(PCIE_0_AUX_CMD_RCGR));
  writel_relaxed(~19, D_REG(PCIE_0_AUX_CMD_RCGR));

  cfg_regval = readl_relaxed(CFG_RCGR_REG(PCIE_0_AUX_CMD_RCGR));
  cfg_regval &= ~(CFG_RCGR_DIV_MASK | CFG_RCGR_SRC_SEL_MASK);
  cfg_regval |= BVAL(4, 0, (int)FIXDIV(1)) | BVAL(10, 8, gcc_xo_source_val);

  /* Activate or disable the M/N:D divider as necessary */
  cfg_regval &= ~MND_MODE_MASK;
  if (n_val != 0)
    cfg_regval |= MND_DUAL_EDGE_MODE_BVAL;
  writel_relaxed(cfg_regval, CFG_RCGR_REG(PCIE_0_AUX_CMD_RCGR));

  // 3. Update RCG config
  rcg_update_config(PCIE_0_AUX_CMD_RCGR);

  // 4. TPL
  gBS->RestoreTPL(OldTpl);
}

VOID EFIAPI pcie_1_aux_clk_set_rate_and_enable(VOID)
{
  EFI_TPL OldTpl;
  u32     cfg_regval;
  u32     n_val = ~(19 - 1) * !!(19);

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Parent clock: __clk_pre_reparent, ignore

  // 2. Set rate
  cfg_regval = readl_relaxed(CFG_RCGR_REG(PCIE_1_AUX_CMD_RCGR));
  writel_relaxed(1, M_REG(PCIE_1_AUX_CMD_RCGR));
  writel_relaxed(n_val, N_REG(PCIE_1_AUX_CMD_RCGR));
  writel_relaxed(~19, D_REG(PCIE_1_AUX_CMD_RCGR));

  cfg_regval = readl_relaxed(CFG_RCGR_REG(PCIE_1_AUX_CMD_RCGR));
  cfg_regval &= ~(CFG_RCGR_DIV_MASK | CFG_RCGR_SRC_SEL_MASK);
  cfg_regval |= BVAL(4, 0, (int)FIXDIV(1)) | BVAL(10, 8, gcc_xo_source_val);

  /* Activate or disable the M/N:D divider as necessary */
  cfg_regval &= ~MND_MODE_MASK;
  if (n_val != 0)
    cfg_regval |= MND_DUAL_EDGE_MODE_BVAL;
  writel_relaxed(cfg_regval, CFG_RCGR_REG(PCIE_1_AUX_CMD_RCGR));

  // 3. Update RCG config
  rcg_update_config(PCIE_1_AUX_CMD_RCGR);

  // 4. TPL
  gBS->RestoreTPL(OldTpl);
}

#define BRANCH_CHECK_MASK BM(31, 28)
#define BRANCH_ON_VAL BVAL(31, 28, 0x0)
#define BRANCH_OFF_VAL BVAL(31, 28, 0x8)
#define BRANCH_NOC_FSM_ON_VAL BVAL(31, 28, 0x2)

VOID EFIAPI branch_clk_halt_check(
    u32 halt_check, void *cbcr_reg, enum branch_state br_status)
{
  char *status_str = (br_status == BRANCH_ON) ? "off" : "on";
  MemoryFence();

  if (halt_check == DELAY || halt_check == HALT_VOTED) {
    udelay(HALT_CHECK_DELAY_US);
  }
  else if (halt_check == HALT) {
    int count;
    u32 val;
    for (count = HALT_CHECK_MAX_LOOPS; count > 0; count--) {
      val = readl_relaxed(cbcr_reg);
      val &= BRANCH_CHECK_MASK;
      switch (br_status) {
      case BRANCH_ON:
        if (val == BRANCH_ON_VAL || val == BRANCH_NOC_FSM_ON_VAL)
          return;
        break;
      case BRANCH_OFF:
        if (val == BRANCH_OFF_VAL)
          return;
        break;
      };
      udelay(1);
    }
    DEBUG((EFI_D_ERROR, "branch_clk_halt_check status stuck \n"));
    CpuDeadLoop();
  }
}

VOID EFIAPI pcie_0_cfg_ahb_clk_enable(VOID)
{
  EFI_TPL OldTpl;
  u32     cbcr_val;

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Enable
  cbcr_val = readl_relaxed(CBCR_REG(PCIE_0_CFG_AHB_CBCR));
  cbcr_val |= CBCR_BRANCH_ENABLE_BIT;
  writel_relaxed(cbcr_val, CBCR_REG(PCIE_0_CFG_AHB_CBCR));

  // 2. Wait for clock to enable before continuing.
  branch_clk_halt_check(HALT, CBCR_REG(PCIE_0_CFG_AHB_CBCR), BRANCH_ON);

  // 3. TPL
  gBS->RestoreTPL(OldTpl);
}

VOID EFIAPI pcie_1_cfg_ahb_clk_enable(VOID)
{
  EFI_TPL OldTpl;
  u32     cbcr_val;

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Enable
  cbcr_val = readl_relaxed(CBCR_REG(PCIE_1_CFG_AHB_CBCR));
  cbcr_val |= CBCR_BRANCH_ENABLE_BIT;
  writel_relaxed(cbcr_val, CBCR_REG(PCIE_1_CFG_AHB_CBCR));

  // 2. Wait for clock to enable before continuing.
  branch_clk_halt_check(HALT, CBCR_REG(PCIE_1_CFG_AHB_CBCR), BRANCH_ON);

  // 3. TPL
  gBS->RestoreTPL(OldTpl);
}

VOID EFIAPI pcie_0_mstr_axi_clk_enable(VOID)
{
  EFI_TPL OldTpl;
  u32     cbcr_val;

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Enable
  cbcr_val = readl_relaxed(CBCR_REG(PCIE_0_MSTR_AXI_CBCR));
  cbcr_val |= CBCR_BRANCH_ENABLE_BIT;
  writel_relaxed(cbcr_val, CBCR_REG(PCIE_0_MSTR_AXI_CBCR));

  // 2. Wait for clock to enable before continuing.
  branch_clk_halt_check(HALT, CBCR_REG(PCIE_0_MSTR_AXI_CBCR), BRANCH_ON);

  // 3. TPL
  gBS->RestoreTPL(OldTpl);
}

VOID EFIAPI pcie_1_mstr_axi_clk_enable(VOID)
{
  EFI_TPL OldTpl;
  u32     cbcr_val;

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Enable
  cbcr_val = readl_relaxed(CBCR_REG(PCIE_1_MSTR_AXI_CBCR));
  cbcr_val |= CBCR_BRANCH_ENABLE_BIT;
  writel_relaxed(cbcr_val, CBCR_REG(PCIE_1_MSTR_AXI_CBCR));

  // 2. Wait for clock to enable before continuing.
  branch_clk_halt_check(HALT, CBCR_REG(PCIE_1_MSTR_AXI_CBCR), BRANCH_ON);

  // 3. TPL
  gBS->RestoreTPL(OldTpl);
}

VOID EFIAPI pcie_0_slv_axi_clk_enable(VOID)
{
  EFI_TPL OldTpl;
  u32     cbcr_val;

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Enable
  cbcr_val = readl_relaxed(CBCR_REG(PCIE_0_SLV_AXI_CBCR));
  cbcr_val |= CBCR_BRANCH_ENABLE_BIT;
  writel_relaxed(cbcr_val, CBCR_REG(PCIE_0_SLV_AXI_CBCR));

  // 2. Wait for clock to enable before continuing.
  branch_clk_halt_check(HALT, CBCR_REG(PCIE_0_SLV_AXI_CBCR), BRANCH_ON);

  // 3. TPL
  gBS->RestoreTPL(OldTpl);
}

VOID EFIAPI pcie_1_slv_axi_clk_enable(VOID)
{
  EFI_TPL OldTpl;
  u32     cbcr_val;

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Enable
  cbcr_val = readl_relaxed(CBCR_REG(PCIE_1_SLV_AXI_CBCR));
  cbcr_val |= CBCR_BRANCH_ENABLE_BIT;
  writel_relaxed(cbcr_val, CBCR_REG(PCIE_1_SLV_AXI_CBCR));

  // 2. Wait for clock to enable before continuing.
  branch_clk_halt_check(HALT, CBCR_REG(PCIE_1_SLV_AXI_CBCR), BRANCH_ON);

  // 3. TPL
  gBS->RestoreTPL(OldTpl);
}

VOID EFIAPI pcie_0_phy_ldo_enable(VOID)
{
  EFI_TPL       OldTpl;
  unsigned long flags;
  u32           regval;

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Enable
  regval = readl_relaxed(GATE_EN_REG(PCIE_0_PHY_LDO_EN));
  regval |= BIT(0);
  writel_relaxed(regval, GATE_EN_REG(PCIE_0_PHY_LDO_EN));

  // 2. Wait
  udelay(100);

  // 3. TPL
  gBS->RestoreTPL(OldTpl);
}

VOID EFIAPI pcie_1_phy_ldo_enable(VOID)
{
  EFI_TPL       OldTpl;
  unsigned long flags;
  u32           regval;

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Enable
  regval = readl_relaxed(GATE_EN_REG(PCIE_1_PHY_LDO_EN));
  regval |= BIT(0);
  writel_relaxed(regval, GATE_EN_REG(PCIE_1_PHY_LDO_EN));

  // 2. Wait
  udelay(100);

  // 3. TPL
  gBS->RestoreTPL(OldTpl);
}

VOID EFIAPI pcie_phy_0_reset_enable(VOID)
{
  EFI_TPL       OldTpl;
  unsigned long flags;
  u32           reg_val;

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Reset
  reg_val = readl_relaxed(BCR_REG(PCIE_PHY_0_BCR));
  // CLK_RESET_DEASSERT
  reg_val &= ~BCR_BLK_ARES_BIT;
  writel_relaxed(reg_val, BCR_REG(PCIE_PHY_0_BCR));

  // 2. TPL
  gBS->RestoreTPL(OldTpl);

  // 3. MB
  MemoryFence();
}

VOID EFIAPI pcie_phy_1_reset_enable(VOID)
{
  EFI_TPL       OldTpl;
  unsigned long flags;
  u32           reg_val;

  // 0. TPL
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  // 1. Reset
  reg_val = readl_relaxed(BCR_REG(PCIE_PHY_1_BCR));
  // CLK_RESET_DEASSERT
  reg_val &= ~BCR_BLK_ARES_BIT;
  writel_relaxed(reg_val, BCR_REG(PCIE_PHY_1_BCR));

  // 2. TPL
  gBS->RestoreTPL(OldTpl);

  // 3. MB
  MemoryFence();
}
