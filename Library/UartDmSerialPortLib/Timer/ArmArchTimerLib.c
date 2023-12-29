/** @file
  Generic ARM implementation of TimerLib.h

  Copyright (c) 2011 - 2021, Arm Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/TimerLib.h>
#include <Library/ArmGenericTimerCounterLib.h>

// Select appropriate multiply function for platform architecture.
#ifdef MDE_CPU_ARM
#define MULT_U64_X_N  MultU64x32
#else
#define MULT_U64_X_N  MultU64x64
#endif

extern UINTN _UartBuiltIn_ArmReadCntFrq(VOID);
extern UINTN _UartBuiltIn_ArmReadCntPct(VOID);

/**
  Stalls the CPU for the number of microseconds specified by MicroSeconds.

  @param  MicroSeconds  The minimum number of microseconds to delay.

  @return The value of MicroSeconds input.

**/
UINTN
EFIAPI
_UartBuiltIn_MicroSecondDelay (
  IN      UINTN  MicroSeconds
  )
{
  UINT64  TimerTicks64;
  UINT64  SystemCounterVal;

  // Calculate counter ticks that represent requested delay:
  //  = MicroSeconds x TICKS_PER_MICRO_SEC
  //  = MicroSeconds x Frequency.10^-6
  TimerTicks64 = DivU64x32 (
                   MULT_U64_X_N (
                     MicroSeconds,
                     _UartBuiltIn_ArmReadCntFrq()
                     ),
                   1000000U
                   );

  // Read System Counter value
  SystemCounterVal = _UartBuiltIn_ArmReadCntPct();

  TimerTicks64 += SystemCounterVal;

  // Wait until delay count expires.
  while (SystemCounterVal < TimerTicks64) {
    SystemCounterVal = _UartBuiltIn_ArmReadCntPct();
  }

  return MicroSeconds;
}
