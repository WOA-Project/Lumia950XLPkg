/*
 * Copyright (c) 2013, Sony Mobile Communications AB.
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <PiDxe.h>
#include <Library/LKEnvLib.h>
#include <Library/InterruptsLib.h>
#include <Library/BitmapLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/QcomGpioTlmmInterrupt.h>

#include "gpio_driver_p.h"

#define MAX_NR_GPIO 300

typedef struct _GPIO_PIN_DATA GPIO_PIN_DATA;
struct _GPIO_PIN_DATA {
  GPIO_INTERRUPT_HANDLER Handler;
  GPIO_INTERRUPT_HANDLER CustomHandler;
};

STATIC UINTN dual_edge_irqs[BITMAP_NUM_WORDS(MAX_NR_GPIO)] = {0};
STATIC UINTN enabled_irqs[BITMAP_NUM_WORDS(MAX_NR_GPIO)] = {0};
STATIC GPIO_PIN_DATA pindata[MAX_NR_GPIO];

STATIC inline UINTN gpio_tlmm_platform_intr_cfg_reg(UINTN id) {
  return PcdGet64 (PcdGpioTlmmIntrCfgOffset) + PcdGet64 (PcdGpioTlmmIntrCfgElementSize) * id;
}

STATIC inline UINTN gpio_tlmm_platform_intr_status_reg(UINTN id) {
  return PcdGet64 (PcdGpioTlmmIntrStatusOffset) + PcdGet64 (PcdGpioTlmmIntrStatusElementSize) * id;
}

STATIC inline UINTN gpio_tlmm_platform_intr_target_reg(UINTN id) {
  return PcdGet64 (PcdGpioTlmmIntrTargetOffset) + PcdGet64 (PcdGpioTlmmIntrTargetElementSize) * id;
}

STATIC inline UINTN gpio_tlmm_platform_io_reg(UINTN id) {
  return PcdGet64 (PcdGpioTlmmIoOffset) + PcdGet64 (PcdGpioTlmmIoElementSize) * id;
}

STATIC VOID msm_gpio_update_dual_edge_pos(UINTN id) {
  INTN loop_limit = 100;
  UINTN val, val2, intstat;
  UINTN pol;

  do {
    val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_io_reg(id)) & BIT(PcdGet64 (PcdGpioTlmmInBit));

    pol = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_cfg_reg(id));
    pol ^= BIT(PcdGet64 (PcdGpioTlmmIntrPolarityBit));
    writel(pol, PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_cfg_reg(id));

    val2 = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_io_reg(id)) & BIT(PcdGet64 (PcdGpioTlmmInBit));
    intstat = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_status_reg(id));
    if (intstat || (val == val2))
      return;
  } while (loop_limit-- > 0);

  DEBUG((EFI_D_ERROR, "dual-edge irq failed to stabilize, %08x != %08x\n", val, val2));
}

VOID msm_gpio_irq_mask(UINTN id) {
  EFI_TPL OldTpl;
  UINT32 val;

  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_cfg_reg(id));
  val &= ~BIT(PcdGet64 (PcdGpioTlmmIntrEnableBit));
  writel(val, PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_cfg_reg(id));

  BitmapClear(enabled_irqs, id);

  gBS->RestoreTPL(OldTpl);
}

VOID msm_gpio_irq_unmask(UINTN id) {
  EFI_TPL OldTpl;
  UINT32 val;

  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_cfg_reg(id));
  val |= BIT(PcdGet64 (PcdGpioTlmmIntrEnableBit));
  writel(val, PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_cfg_reg(id));

  BitmapSet(enabled_irqs, id);

  gBS->RestoreTPL(OldTpl);
}

VOID msm_gpio_irq_ack(UINTN id) {
  EFI_TPL OldTpl;
  UINT32 val;

  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_status_reg(id));
  if (PcdGetBool (PcdGpioTlmmIntrAckHigh))
    val |= BIT(PcdGet64 (PcdGpioTlmmIntrStatusBit));
  else
    val &= ~BIT(PcdGet64 (PcdGpioTlmmIntrStatusBit));
  writel(val, PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_status_reg(id));

  if (BitmapTest(dual_edge_irqs, id))
    msm_gpio_update_dual_edge_pos(id);

  gBS->RestoreTPL(OldTpl);
}

STATIC VOID handle_level_irq(UINTN id) {
  DEBUG((EFI_D_ERROR, "%a gpio=%u\n", __func__, id));
  ASSERT(FALSE);
}

STATIC VOID handle_edge_irq(UINTN id) {
  DEBUG((EFI_D_ERROR, "%a gpio=%u\n", __func__, id));
  ASSERT(FALSE);
}

EFI_STATUS msm_gpio_irq_set_type(UINTN id, UINTN type) {
  EFI_TPL OldTpl;
  UINT32 val;

  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  //
  // For hw without possibility of detecting both edges
  //
  if (PcdGet64 (PcdGpioTlmmIntrDetectionWidth) == 1 && type == TLMM_IRQ_TYPE_EDGE_BOTH)
    BitmapSet(dual_edge_irqs, id);
  else
    BitmapClear(dual_edge_irqs, id);

  // Route interrupts to application cpu
  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_target_reg(id));
  val &= ~(7 << PcdGet64 (PcdGpioTlmmIntrTargetBit));
  val |= PcdGet64 (PcdGpioTlmmIntrTargetKpssValue) << PcdGet64 (PcdGpioTlmmIntrTargetBit);
  writel(val, PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_target_reg(id));

  //
  // Update configuration for gpio.
  // RAW_STATUS_EN is left on for all gpio irqs. Due to the
  // internal circuitry of TLMM, toggling the RAW_STATUS
  // could cause the INTR_STATUS to be set for EDGE interrupts.
  //
  val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_cfg_reg(id));
  val |= BIT(PcdGet64 (PcdGpioTlmmIntrRawStatusBit));
  if (PcdGet64 (PcdGpioTlmmIntrDetectionWidth) == 2) {
    val &= ~(3 << PcdGet64 (PcdGpioTlmmIntrDetectionBit));
    val &= ~(1 << PcdGet64 (PcdGpioTlmmIntrPolarityBit));
    switch (type) {
      case TLMM_IRQ_TYPE_EDGE_RISING:
        val |= 1 << PcdGet64 (PcdGpioTlmmIntrDetectionBit);
        val |= BIT(PcdGet64 (PcdGpioTlmmIntrPolarityBit));
        break;
      case TLMM_IRQ_TYPE_EDGE_FALLING:
        val |= 2 << PcdGet64 (PcdGpioTlmmIntrDetectionBit);
        val |= BIT(PcdGet64 (PcdGpioTlmmIntrPolarityBit));
        break;
      case TLMM_IRQ_TYPE_EDGE_BOTH:
        val |= 3 << PcdGet64 (PcdGpioTlmmIntrDetectionBit);
        val |= BIT(PcdGet64 (PcdGpioTlmmIntrPolarityBit));
        break;
      case TLMM_IRQ_TYPE_LEVEL_LOW:
        break;
      case TLMM_IRQ_TYPE_LEVEL_HIGH:
        val |= BIT(PcdGet64 (PcdGpioTlmmIntrPolarityBit));
        break;
    }
  }
  else if (PcdGet64 (PcdGpioTlmmIntrDetectionWidth) == 1) {
    val &= ~(1 << PcdGet64 (PcdGpioTlmmIntrDetectionBit));
    val &= ~(1 << PcdGet64 (PcdGpioTlmmIntrPolarityBit));
    switch (type) {
      case TLMM_IRQ_TYPE_EDGE_RISING:
        val |= BIT(PcdGet64 (PcdGpioTlmmIntrDetectionBit));
        val |= BIT(PcdGet64 (PcdGpioTlmmIntrPolarityBit));
        break;
      case TLMM_IRQ_TYPE_EDGE_FALLING:
        val |= BIT(PcdGet64 (PcdGpioTlmmIntrDetectionBit));
        break;
      case TLMM_IRQ_TYPE_EDGE_BOTH:
        val |= BIT(PcdGet64 (PcdGpioTlmmIntrDetectionBit));
        val |= BIT(PcdGet64 (PcdGpioTlmmIntrPolarityBit));
        break;
      case TLMM_IRQ_TYPE_LEVEL_LOW:
        break;
      case TLMM_IRQ_TYPE_LEVEL_HIGH:
        val |= BIT(PcdGet64 (PcdGpioTlmmIntrPolarityBit));
        break;
    }
  }
  else {
    gBS->RestoreTPL(OldTpl);
    return EFI_INVALID_PARAMETER;
  }
  writel(val, PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_cfg_reg(id));

  if (BitmapTest(dual_edge_irqs, id))
    msm_gpio_update_dual_edge_pos(id);

  if (type & (TLMM_IRQ_TYPE_LEVEL_LOW | TLMM_IRQ_TYPE_LEVEL_HIGH))
    pindata[id].Handler = handle_level_irq;
  else if (type & (TLMM_IRQ_TYPE_EDGE_FALLING | TLMM_IRQ_TYPE_EDGE_RISING))
    pindata[id].Handler = handle_edge_irq;

  gBS->RestoreTPL(OldTpl);

  return EFI_SUCCESS;
}

VOID msm_gpio_irq_set_custom_handler(UINTN id, GPIO_INTERRUPT_HANDLER Handler) {
  pindata[id].CustomHandler = Handler;
}

EFI_STATUS msm_gpio_irq_set_wake(UINTN on) {
  if (on)
    unmask_interrupt(PcdGet64 (PcdGpioTlmmSummaryIrq));
  else
    mask_interrupt(PcdGet64 (PcdGpioTlmmSummaryIrq));

  return EFI_SUCCESS;
}

STATIC enum handler_return msm_gpio_irq_handler(VOID *arg)
{
  INTN handled = 0;
  UINT32 val;
  INTN i;

  //
  // Each pin has it's own IRQ status register, so use
  // enabled_irq bitmap to limit the number of reads.
  //
  for(i=0; i<MAX_NR_GPIO; i++) {
    if (!BitmapTest(enabled_irqs, i))
      continue;

    val = readl(PcdGet64 (PcdGpioTlmmBaseAddress) + gpio_tlmm_platform_intr_status_reg(i));
    if (val & BIT(PcdGet64 (PcdGpioTlmmIntrStatusBit))) {
      if (pindata[i].CustomHandler)
        pindata[i].CustomHandler(i);
      else if (pindata[i].Handler)
        pindata[i].Handler(i);
      handled++;
    }
  }

  // No interrupts were flagged
  if (handled == 0) {
    DEBUG((EFI_D_ERROR, "unexpected GPIO IRQ\n"));
  }

  return INT_RESCHEDULE;
}

VOID msm_gpio_irq_init(VOID) {
  INTN i;

  for(i=0; i<ARRAY_SIZE(pindata); i++) {
    pindata[i].Handler = NULL;
    pindata[i].CustomHandler = NULL;
  }

  register_int_handler(PcdGet64 (PcdGpioTlmmSummaryIrq), msm_gpio_irq_handler, 0);
  msm_gpio_irq_set_wake(1);
}
