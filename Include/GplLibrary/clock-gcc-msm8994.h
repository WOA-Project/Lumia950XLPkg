#ifndef __CLOCK_GCC_MSM8994_H__
#define __CLOCK_GCC_MSM8994_H__

VOID EFIAPI pcie_1_pipe_clk_set_rate_enable(VOID);

VOID EFIAPI pcie_1_aux_clk_set_rate_and_enable(VOID);

VOID EFIAPI pcie_1_cfg_ahb_clk_enable(VOID);

VOID EFIAPI pcie_1_mstr_axi_clk_enable(VOID);

VOID EFIAPI pcie_1_slv_axi_clk_enable(VOID);

VOID EFIAPI pcie_1_phy_ldo_enable(VOID);

VOID EFIAPI pcie_phy_1_reset_enable(VOID);

VOID EFIAPI pcie_0_pipe_clk_set_rate_enable(VOID);

VOID EFIAPI pcie_0_aux_clk_set_rate_and_enable(VOID);

VOID EFIAPI pcie_0_cfg_ahb_clk_enable(VOID);

VOID EFIAPI pcie_0_mstr_axi_clk_enable(VOID);

VOID EFIAPI pcie_0_slv_axi_clk_enable(VOID);

VOID EFIAPI pcie_0_phy_ldo_enable(VOID);

VOID EFIAPI pcie_phy_0_reset_enable(VOID);

#endif