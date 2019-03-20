#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <GplLibrary/gdsc-msm8994.h>

#define PWR_ON_MASK BIT(31)
#define EN_REST_WAIT_MASK (0xF << 20)
#define EN_FEW_WAIT_MASK (0xF << 16)
#define CLK_DIS_WAIT_MASK (0xF << 12)
#define SW_OVERRIDE_MASK BIT(2)
#define HW_CONTROL_MASK BIT(1)
#define SW_COLLAPSE_MASK BIT(0)
#define GMEM_CLAMP_IO_MASK BIT(0)

/* Wait 2^n CXO cycles between all states. Here, n=2 (4 cycles). */
#define EN_REST_WAIT_VAL (0x2 << 20)
#define EN_FEW_WAIT_VAL (0x8 << 16)
#define CLK_DIS_WAIT_VAL (0x2 << 12)

#define TIMEOUT_US 100
#define MAX_GDSCR_READS 100

#define GDSC_PCIE0 (VOID *)(UINTN) 0xfc401ac4
#define GDSC_PCIE1 (VOID *)(UINTN) 0xfc401b44
#define ETIMEDOUT 110

enum gdscr_status {
  ENABLED,
  DISABLED,
};

static int poll_gdsc_status(void *gdscr, enum gdscr_status status)
{
  int count;
  u32 val;
  for (count = MAX_GDSCR_READS; count > 0; count--) {
    val = readl_relaxed(gdscr);
    val &= PWR_ON_MASK;
    switch (status) {
    case ENABLED:
      if (val)
        return 0;
      break;
    case DISABLED:
      if (!val)
        return 0;
      break;
    }
    /*
     * There is no guarantee about the delay needed for the enable
     * bit in the GDSCR to be set or reset after the GDSC state
     * changes. Hence, keep on checking for a reasonable number
     * of times until the bit is set with the least possible delay
     * between succeessive tries.
     */
    udelay(1);
  }
  return -ETIMEDOUT;
}

VOID EFIAPI gdsc_pcie0_enable(VOID)
{
  uint32_t regval;
  int      ret;

  regval = readl_relaxed(GDSC_PCIE0);
  regval &= ~(HW_CONTROL_MASK | SW_OVERRIDE_MASK);

  /* Configure wait time between states. */
  regval &= ~(EN_REST_WAIT_MASK | EN_FEW_WAIT_MASK | CLK_DIS_WAIT_MASK);
  regval |= EN_REST_WAIT_VAL | EN_FEW_WAIT_VAL | CLK_DIS_WAIT_VAL;
  writel_relaxed(regval, GDSC_PCIE0);
  MemoryFence();

  /* Enable logic */
  regval = readl_relaxed(GDSC_PCIE0);
  if (regval & HW_CONTROL_MASK) {
    DEBUG((EFI_D_ERROR, "GDSC PCIE0 Under HW control \n"));
    CpuDeadLoop();
  }

  regval &= ~SW_COLLAPSE_MASK;
  writel_relaxed(regval, GDSC_PCIE0);
  ret = poll_gdsc_status(GDSC_PCIE0, ENABLED);

  if (ret) {
    DEBUG((EFI_D_ERROR, "GDSC PCIE0 enable timed out \n"));
    udelay(TIMEOUT_US);
    regval = readl_relaxed(GDSC_PCIE0);
    DEBUG(
        (EFI_D_ERROR, "GDSC PCIE0 final state: 0x%x (%d us after timeout)\n",
         regval, TIMEOUT_US));
    CpuDeadLoop();
  }

  /*
   * If clocks to this power domain were already on, they will take an
   * additional 4 clock cycles to re-enable after the rail is enabled.
   * Delay to account for this. A delay is also needed to ensure clocks
   * are not enabled within 400ns of enabling power to the memories.
   */
  udelay(1);
}

VOID EFIAPI gdsc_pcie1_enable(VOID)
{
  uint32_t regval;
  int      ret;

  regval = readl_relaxed(GDSC_PCIE1);
  regval &= ~(HW_CONTROL_MASK | SW_OVERRIDE_MASK);

  /* Configure wait time between states. */
  regval &= ~(EN_REST_WAIT_MASK | EN_FEW_WAIT_MASK | CLK_DIS_WAIT_MASK);
  regval |= EN_REST_WAIT_VAL | EN_FEW_WAIT_VAL | CLK_DIS_WAIT_VAL;
  writel_relaxed(regval, GDSC_PCIE1);
  MemoryFence();

  /* Enable logic */
  regval = readl_relaxed(GDSC_PCIE1);
  if (regval & HW_CONTROL_MASK) {
    DEBUG((EFI_D_ERROR, "GDSC PCIE1 Under HW control \n"));
    CpuDeadLoop();
  }

  regval &= ~SW_COLLAPSE_MASK;
  writel_relaxed(regval, GDSC_PCIE1);
  ret = poll_gdsc_status(GDSC_PCIE1, ENABLED);

  if (ret) {
    DEBUG((EFI_D_ERROR, "GDSC PCIE1 enable timed out \n"));
    udelay(TIMEOUT_US);
    regval = readl_relaxed(GDSC_PCIE1);
    DEBUG(
        (EFI_D_ERROR, "GDSC PCIE1 final state: 0x%x (%d us after timeout)\n",
         regval, TIMEOUT_US));
    CpuDeadLoop();
  }

  /*
   * If clocks to this power domain were already on, they will take an
   * additional 4 clock cycles to re-enable after the rail is enabled.
   * Delay to account for this. A delay is also needed to ensure clocks
   * are not enabled within 400ns of enabling power to the memories.
   */
  udelay(1);
}
