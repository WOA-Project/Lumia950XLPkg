#------------------------------------------------------------------------------
#
# Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
# Copyright (c) 2011 - 2014, ARM Limited. All rights reserved.
# Copyright (c) 2016, Linaro Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
#------------------------------------------------------------------------------

#include <AsmMacroIoLibV8.h>

// DAIF bit definitions for writing through msr daifclr/sr daifset
.set DAIF_WR_FIQ_BIT,   (1 << 0)
.set DAIF_WR_IRQ_BIT,   (1 << 1)
.set DAIF_WR_ABORT_BIT, (1 << 2)
.set DAIF_WR_DEBUG_BIT, (1 << 3)
.set DAIF_WR_INT_BITS,  (DAIF_WR_FIQ_BIT | DAIF_WR_IRQ_BIT)
.set DAIF_WR_ALL,       (DAIF_WR_DEBUG_BIT | DAIF_WR_ABORT_BIT | DAIF_WR_INT_BITS)

.set DAIF_RD_FIQ_BIT,   (1 << 6)
.set DAIF_RD_IRQ_BIT,   (1 << 7)

ASM_FUNC(_UartBuiltin_ArmEnableInterrupts)
  msr   daifclr, #DAIF_WR_INT_BITS
  isb
  ret


ASM_FUNC(_UartBuiltin_ArmDisableInterrupts)
  msr   daifset, #DAIF_WR_INT_BITS
  isb
  ret

ASM_FUNC(_UartBuiltin_ArmGetInterruptState)
  mrs     x0, daif
  tst     w0, #DAIF_RD_IRQ_BIT  // Check if IRQ is enabled. Enabled if 0 (Z=1)
  cset    w0, eq                // if Z=1 return 1, else 0
  ret

ASM_FUNCTION_REMOVE_IF_UNREFERENCED
