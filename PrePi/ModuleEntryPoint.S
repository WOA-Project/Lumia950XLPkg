#include <Library/PcdLib.h>
#include <AsmMacroIoLibV8.h>
#include <Chipset/AArch64.h>

#define LoadConstantToReg(Data, Reg) \
  ldr  Reg, =Data

.text
.align 3

GCC_ASM_IMPORT (CEntryPoint)
GCC_ASM_IMPORT (ArmWriteCpacr)
GCC_ASM_IMPORT (ArmEnableInstructionCache)
GCC_ASM_IMPORT (ArmEnableDataCache)
GCC_ASM_IMPORT (ArmInvalidateTlb)

GCC_ASM_EXPORT (_ModuleEntryPoint)

.global _StackBase 
.global _StackSize
.global _SecondaryCpuStackSize
.global _MpParkBase

_StackBase:
  .quad FixedPcdGet32(PcdPrePiStackBase)

_StackSize:
  .quad FixedPcdGet32(PcdPrePiStackSize)

_SecondaryCpuStackSize:
  .quad FixedPcdGet32(SecondaryCpuStackSize)

_MpParkBase:
  .quad FixedPcdGet64(SecondaryCpuMpParkRegionBase)

_ModuleEntryPoint:
  mov x0, #0

  /* Know about the current CPU index */
  mrs x3, mpidr_el1
  mov x2, x3
  mov x1, #0x0000100
  and x2, x2, x1
  cmp x2, #0x0
  mov x0, #0
  beq _CpuIndex

  mov x0, #4
_CpuIndex:
  mov x2, x3
  mov x1, #0xf
  and x2, x2, x1
  add x0, x0, x2

  /* Keep the CPU index in x19 */
  mov x19, x0

  /* Secondary startup is not expected */
  cmp x19, #0x0
  bne dead

  /* First ensure all interrupts are disabled */
  bl ASM_PFX(ArmDisableInterrupts)

  /* Ensure that the MMU and caches are off */
  bl ASM_PFX(ArmDisableCachesAndMmu)
 
  /* Invalide I-Cache */
  bl ASM_PFX(ArmInvalidateInstructionCache)
  
  /* Invalidate TLB */
  bl ASM_PFX(ArmInvalidateTlb)

_SetupExceptionVector:
  LoadConstantToReg (FixedPcdGet32(PcdCpuVectorBaseAddress), x0)
  ldr x1, dead
  mov x2, #0

_FillVectors:
  /* Each entry is 8 bytes, vector table size is 0x800 per core */
  str     x1, [x0, x2]
  adds    x2, x2, #8
  cmp     x2, #0x800
  bne     _FillVectors

  /* Get current EL in x0 */
  mrs x0, CurrentEl

  /* Check if we are in EL1 */
  cmp x0, #0x4
  b.eq _SetupEl1VBar

  cmp x0, #0x5
  b.eq _SetupEl1VBar
  
_SetupEl1VBar:
  msr vbar_el1, x0        
    
_DonNotTrap_VFP_SIMD:
  mrs x0, CPACR_EL1
  /* Bit 20 and 21 */
  orr x0, x0, #0x300000
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
