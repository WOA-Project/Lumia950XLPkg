.set MPIDR_U_BIT,    (30)
.set MPIDR_U_MASK,   (1 << MPIDR_U_BIT)

// CurrentEL : 0xC = EL3; 8 = EL2; 4 = EL1
// This only selects between EL1 and EL2 and EL3, else we die.
// Provide the Macro with a safe temp xreg to use.
#define EL1_OR_EL2_OR_EL3(SAFE_XREG) \
        mrs    SAFE_XREG, CurrentEL ;\
        cmp    SAFE_XREG, #0x8      ;\
        b.gt   3f                   ;\
        b.eq   2f                   ;\
        cbnz   SAFE_XREG, 1f        ;\
        b      .                    ;// We should never get here

// DAIF bit definitions for writing through msr daifclr/sr daifset
.set DAIF_WR_FIQ_BIT,   (1 << 0)
.set DAIF_WR_IRQ_BIT,   (1 << 1)
.set DAIF_WR_ABORT_BIT, (1 << 2)
.set DAIF_WR_DEBUG_BIT, (1 << 3)
.set DAIF_WR_INT_BITS,  (DAIF_WR_FIQ_BIT | DAIF_WR_IRQ_BIT)
.set DAIF_WR_ALL,       (DAIF_WR_DEBUG_BIT | DAIF_WR_ABORT_BIT | DAIF_WR_INT_BITS)

.set CTRL_M_BIT,      (1 << 0)
.set CTRL_A_BIT,      (1 << 1)
.set CTRL_C_BIT,      (1 << 2)
.set CTRL_SA_BIT,     (1 << 3)
.set CTRL_I_BIT,      (1 << 12)
.set CTRL_V_BIT,      (1 << 12)
.set CPACR_VFP_BITS,  (3 << 20)

// ArmDisableInterrupts
msr   daifset, #DAIF_WR_INT_BITS
isb

// ArmDisableCachesAndMmu
   EL1_OR_EL2_OR_EL3(x1)
1: mrs     x0, sctlr_el1        // Get control register EL1
   b       4f
2: mrs     x0, sctlr_el2        // Get control register EL2
   b       4f
3: mrs     x0, sctlr_el3        // Get control register EL3
4: mov     x1, #~(CTRL_M_BIT | CTRL_C_BIT | CTRL_I_BIT)  // Disable MMU, D & I caches
   and     x0, x0, x1
   EL1_OR_EL2_OR_EL3(x1)
1: msr     sctlr_el1, x0        // Write back control register
   b       4f
2: msr     sctlr_el2, x0        // Write back control register
   b       4f
3: msr     sctlr_el3, x0        // Write back control register
4: dsb     sy
   isb

// Load address and branch
mov x0, #0x00200000
br x0