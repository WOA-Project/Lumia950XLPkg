/** @file
This files implements early GIC initialization.

Copyright (c) 2018, Bingxing Wang. All rights reserved.
Copyright (c) 2016, Brian McKenzie. All rights reserved.
Copyright (c) 2015-2016, Linaro Limited. All rights reserved.
Copyright (c) 2015-2016, Hisilicon Limited. All rights reserved.

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD
License which accompanies this distribution.  The full text of the license may
be found at http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Pi.h"

#include <Guid/LzmaDecompress.h>
#include <Guid/VariableFormat.h>
#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <PiDxe.h>
#include <PiPei.h>
#include <Ppi/GuidedSectionExtraction.h>

#include <Library/ArmGicLib.h>
#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/DebugLib.h>
#include <Library/FrameBufferSerialPortLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PrePiHobListPointerLib.h>
#include <Library/PrePiLib.h>
#include <Library/SerialPortLib.h>

#include "EarlyQGic.h"

STATIC
UINT8
QgicGetCpuMask(VOID)
{
  UINT32 mask = 0, i;

  /* Fetch the CPU MASK from the SGI/PPI reg */
  for (i = 0; i < 32; i += 4) {
    mask = MmioRead32(GIC_DIST_TARGET + i);
    mask |= mask >> 16;
    mask |= mask >> 8;
    if (mask) {
      break;
    }
  }
  return mask;
}

/* Intialize distributor */
VOID QGicDistConfig(UINT32 NumIrq)
{
  UINT32 i;

  /* Set each interrupt line to use N-N software model
   * and edge sensitive, active high
   */
  for (i = 32; i < NumIrq; i += 16) {
    MmioWrite32(GIC_DIST_CONFIG + i * 4 / 16, 0xffffffff);
  }

  MmioWrite32(GIC_DIST_CONFIG + 4, 0xffffffff);

  /* Set priority of all interrupts */

  /*
   * In bootloader we dont care about priority so
   * setting up equal priorities for all
   */
  for (i = 0; i < NumIrq; i += 4) {
    MmioWrite32(GIC_DIST_PRI + i * 4 / 4, 0xa0a0a0a0);
  }

  /* Disabling interrupts */
  for (i = 0; i < NumIrq; i += 32) {
    MmioWrite32(GIC_DIST_ENABLE_CLEAR + i * 4 / 32, 0xffffffff);
  }

  MmioWrite32(GIC_DIST_ENABLE_SET, 0x0000ffff);
}

VOID QGicHardwareReset(VOID)
{
  UINT32 n;

  /* Disabling GIC */
  MmioWrite32(GIC_DIST_CTRL, 0);
  MmioWrite32(GIC_DIST_CGCR, 0);
  MmioWrite32(GIC_CPU_CTRL, 0);
  MmioWrite32(GIC_CPU_PRIMASK, 0);
  MmioWrite32(GIC_CPU_BINPOINT, 0);

  for (n = 0; n <= 11; n++) {
    MmioWrite32(GIC_DIST_REG(0x80 + 4 * n), 0);
    MmioWrite32(GIC_DIST_REG(0x180 + 4 * n), 0xFFFFFFFF);
    MmioWrite32(GIC_DIST_REG(0x280 + 4 * n), 0xFFFFFFFF);
  }

  for (n = 0; n <= 95; n++) {
    MmioWrite32(GIC_DIST_REG(0x400 + 4 * n), 0);
    MmioWrite32(GIC_DIST_REG(0x800 + 4 * n), 0);
  }

  for (n = 0; n <= 23; n++) {
    MmioWrite32(GIC_DIST_REG(0xc00 + 4 * n), 0);
  }
}

VOID QGicSetBinpoint(VOID)
{
  /* No PREEMPT */
  MmioWrite32(GIC_CPU_BINPOINT, 7);
}

VOID QGicDistInit(VOID)
{
  UINT32 i;
  UINT32 num_irq = 0;
  UINT32 cpumask;

  cpumask = QgicGetCpuMask();
  cpumask |= cpumask << 8;
  cpumask |= cpumask << 16;

  /* Find out how many interrupts are supported. */
  num_irq = MmioRead32(GIC_DIST_CTR) & 0x1f;
  num_irq = (num_irq + 1) * 32;

  /* Set up interrupts for this CPU */
  for (i = 32; i < num_irq; i += 4) {
    MmioWrite32(GIC_DIST_TARGET + i * 4 / 4, cpumask);
  }

  QGicDistConfig(num_irq);

  /*Enabling GIC */
  MmioWrite32(GIC_DIST_CTRL, 1);
}

/* Intialize cpu specific controller */
VOID QGicCpuInit(VOID)
{
  MmioWrite32(GIC_CPU_PRIMASK, 0xfe);
  MmioWrite32(GIC_CPU_CTRL, 1);
}

EFI_STATUS
EFIAPI
QGicPeim(VOID)
{
  QGicHardwareReset();
  QGicSetBinpoint();
  QGicDistInit();
  QGicCpuInit();

  return EFI_SUCCESS;
}

UINTN
EFIAPI
ArmGicV2AcknowledgeInterrupt(IN UINTN GicInterruptInterfaceBase)
{
  // Read the Interrupt Acknowledge Register
  return MmioRead32(GicInterruptInterfaceBase + ARM_GIC_ICCIAR);
}

UINTN
EFIAPI
ArmGicAcknowledgeInterrupt(
    IN UINTN GicInterruptInterfaceBase, OUT UINTN *InterruptId)
{
  UINTN Value;

  Value = ArmGicV2AcknowledgeInterrupt(GicInterruptInterfaceBase);
  if (InterruptId != NULL) {
    *InterruptId = Value & ARM_GIC_ICCIAR_ACKINTID;
  }

  return Value;
}

VOID EFIAPI ArmGicSendSgiTo(
    IN INTN GicDistributorBase, IN INTN TargetListFilter, IN INTN CPUTargetList,
    IN INTN SgiId)
{
  MmioWrite32(
      GicDistributorBase + ARM_GIC_ICDSGIR, ((TargetListFilter & 0x3) << 24) |
                                                ((CPUTargetList & 0xFF) << 16) |
                                                SgiId);
}

VOID EFIAPI ArmGicV2EnableInterruptInterface(IN INTN GicInterruptInterfaceBase)
{
  /*
   * Enable the CPU interface in Non-Secure world
   * Note: The ICCICR register is banked when Security extensions are
   * implemented
   */
  MmioWrite32(GicInterruptInterfaceBase + ARM_GIC_ICCICR, 0x1);
}

VOID EFIAPI ArmGicEnableInterruptInterface(IN INTN GicInterruptInterfaceBase)
{
  ArmGicV2EnableInterruptInterface(GicInterruptInterfaceBase);
}

VOID EFIAPI
     ArmGicV2EndOfInterrupt(IN UINTN GicInterruptInterfaceBase, IN UINTN Source)
{
  MmioWrite32(GicInterruptInterfaceBase + ARM_GIC_ICCEIOR, Source);
}

UINTN
EFIAPI
ArmGicGetMaxNumInterrupts(IN INTN GicDistributorBase)
{
  return 32 * ((MmioRead32(GicDistributorBase + ARM_GIC_ICDICTR) & 0x1F) + 1);
}

VOID EFIAPI
     ArmGicEndOfInterrupt(IN UINTN GicInterruptInterfaceBase, IN UINTN Source)
{
  ArmGicV2EndOfInterrupt(GicInterruptInterfaceBase, Source);
}
