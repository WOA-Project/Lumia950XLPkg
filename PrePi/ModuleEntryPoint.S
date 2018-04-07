//------------------------------------------------------------------------------ 
//
//  Copyright (c) 2012-2014, Qualcomm Technologies Inc. All rights reserved.
//  Portions Copyright (c) 2011-2013, ARM Limited. All rights reserved.
//
//  This program and the accompanying materials
//  are licensed and made available under the terms and conditions of the BSD License
//  which accompanies this distribution.  The full text of the license may be found at
//  http://opensource.org/licenses/bsd-license.php
//
//  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
//  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
//
//------------------------------------------------------------------------------

//=============================================================================
//                              EDIT HISTORY
//
//
// when       who     what, where, why
// --------   ---     ---------------------------------------------------------
// 12/10/14   bh      Invalidate TLB/I-cache earlier, leave data cache after stack setup
// 12/03/14   bh      Invalidate TLB
// 09/18/14   na      Invalidate cache prior to enabling it
// 07/28/14   lm      Add PRE_SIL for 8909
// 07/25/14   na      Do not trap SIMD instructions
// 05/05/14   vk      Setup exception vectors
// 04/30/14   vk      Add PRE_SIL for 8916
// 03/03/14   vk      Disable MMU and interrupts first
// 02/14/14   vk      Initial revision
//
//============================================================================


#include <Library/PcdLib.h>
#include <AsmMacroIoLibV8.h>
#include <Chipset/AArch64.h>

#define LoadConstantToReg(Data, Reg) \
  ldr  Reg, =Data

.text
.align 3

GCC_ASM_IMPORT (CEntryPoint)
GCC_ASM_EXPORT (_ModuleEntryPoint)
GCC_ASM_IMPORT (ArmWriteCpacr)
GCC_ASM_IMPORT (ArmEnableInstructionCache)
GCC_ASM_IMPORT (ArmEnableDataCache)
GCC_ASM_IMPORT (ArmInvalidateTlb)

.global _StackBase 
.global _StackSize
.global CNTFRQ 

_StackBase:
  .quad FixedPcdGet32(PcdPrePiStackBase)

_StackSize:
  .quad FixedPcdGet32(PcdPrePiStackSize)

CNTFRQ:
  .quad 0x0124F800

_ModuleEntryPoint:
  mov x0, #0

  /* First ensure all interrupts are disabled */
  bl ASM_PFX(ArmDisableInterrupts)

  /* Ensure that the MMU and caches are off */
  bl ASM_PFX(ArmDisableCachesAndMmu)
 
  /* Invalide I-Cache */
  bl ASM_PFX(ArmInvalidateInstructionCache)
  
  /* Invalidate TLB */
  bl ASM_PFX(ArmInvalidateTlb)
  
  /* Get current EL in x0 */
  mrs x0, CurrentEl

  /* Check if we are in EL1, if yes skip other EL init */
  cmp x0, #0x4
  b.eq _Start_EL1_NS

  cmp x0, #0x5
  b.eq _Start_EL1_NS

  /* We should have EL1 initialized */
  b dead

_Start_EL1_NS:
_SetupExceptionVector:
  LoadConstantToReg (FixedPcdGet32(PcdCpuVectorBaseAddress), x0)
  ldr x1, dead
  mov x2, #0

_FillVectors:
  str     x1, [x0, x2]
  adds    x2, x2, #8      /* Fill every 8 byte */
  cmp     x2, #0x800      /* Vector Table Size */
  bne     _FillVectors
  
  /* Update VBAR */
  msr vbar_el1, x0        /* End _SetupExceptionVector */
    
_DonNotTrap_VFP_SIMD:
  mrs x0, CPACR_EL1
  orr x0, x0, #0x300000  /* Set FPEN Bits 20 and 21 for not trapping FP and Advanced SIMD instructions */
  msr CPACR_EL1, x0
    
_SetupPrimaryCoreStack:
  ldr x0, _StackBase     /* Stack base arg0 */
  ldr x1, _StackSize     /* Stack size arg1 */
  
  add x4, x0, x1
  add sp, x4, #0

_EnableCache: 
  bl ArmInvalidateDataCache
  bl ASM_PFX(ArmEnableInstructionCache)
  bl ASM_PFX(ArmEnableDataCache)

_PrepareArguments:
  /* x0 = _StackBase and x1 = _StackSize */
  ldr x0, _StackBase     /* Stack base arg0 */
  ldr x1, _StackSize     /* Stack size arg1 */

  bl CEntryPoint

.align 3
dead:  
  b dead                      /* We should never get here */

