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

