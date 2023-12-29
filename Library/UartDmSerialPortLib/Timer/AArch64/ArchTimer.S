#------------------------------------------------------------------------------
#
# Copyright (c) 2011 - 2013, ARM Limited. All rights reserved.
# Copyright (c) 2016, Linaro Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
#------------------------------------------------------------------------------

#include <AsmMacroIoLibV8.h>

ASM_FUNC(_UartBuiltIn_ArmReadCntFrq)
  mrs   x0, cntfrq_el0           // Read CNTFRQ
  ret

ASM_FUNC(_UartBuiltIn_ArmReadCntPct)
  mrs   x0, cntpct_el0           // Read CNTPCT (Physical counter register)
  ret

ASM_FUNCTION_REMOVE_IF_UNREFERENCED
