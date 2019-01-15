//============================================================================
//
//                 TZBSPTEST ASM FUNCTIONS
//
// Copyright (c) 2013-2014 Copyright Qualcomm Technologies, Inc.  All Rights Reserved.
// Qualcomm Technologies Proprietary and Confidential.
//   
//============================================================================

//============================================================================
//
//                           EDIT HISTORY FOR FILE
//
// This section contains comments describing changes made to the module.
// Notice that changes are listed in reverse chronological order.
//
//
// when       who     what, where, why
// --------   ---     --------------------------------------------------------
// 04/17/14   sm      Resolved compilie time optimization issues.
// 11/15/13   sm      Initial revision. Branched from A32 version
//============================================================================

.text
.align 3

//============================================================================
//                             MODULE EXPORTS
//============================================================================

  GCC_ASM_EXPORT(tz_fastcall)
  GCC_ASM_EXPORT(tz_slowcall)
  GCC_ASM_EXPORT(tz_armv8_smc_call)
  GCC_ASM_EXPORT(tz_get_version)

//============================================================================
//
//                        MACROS AND FUNCTIONS
//
//============================================================================

#define func_prologue()                     \
    stp           x29, x30, [sp, #-0x10]!;  \
    mov           x29,sp


#define func_epilogue()                     \
    ldp           x29, x30, [sp], #0x10  ;  \
    ret

#define smc_call(func)                \
smc_loop_##func:                  ;   \
    smc           #0x0            ;   \
    cmp           w0, #0          ;   \
    b.gt          smc_loop_##func



//============================================================================
//
//  int tz_get_version(uint32* version)//
//
//============================================================================
tz_get_version:
    func_prologue()
    str           x21, [sp, #-0x10]!
    mov           x21, x0                 // Save pointer to version
    orr           w0, wzr, #0x100         // val 1 in bits 8-11 means get_version
    smc_call(get_version)
    cbnz          x0, tz_get_version_end  // If request successful,
    str           w1, [x21, #0]           // then store the version to argument ptr.
tz_get_version_end:
    ldr           x21, [sp], #0x10
    func_epilogue()


//============================================================================
//
//  int tz_slowcall(const uint32* cmd_addr)//
//      r0 - Is fixed to argument count 1
//      r1 - Unique context id (stack pointer will do)
//      r2 - Pointer to command memory area
//
//============================================================================
tz_slowcall:
    func_prologue()
    mov           x2, x0
    mov           w0, #1
    mov           w1, wsp
    smc_call(slowcall)
    func_epilogue()

//============================================================================
//
//  int tz_fastcall(uint32 id, uint32 token, uint32 r0, uint32 r1, ...)//
//      r0  - id, TZ SYSCALL id, lowest nibble contains the number of arguments.
//      r1  - token, SMP token.
//      r2  - First argument to TZ syscall (R0, optional).
//      r3  - Second argument to TZ syscall (R1, optional).
//      r4  - Third argument to TZ syscall.
//      r5  - Fourth argument to TZ syscall.
//      r6  - Fifth argument to TZ syscall.
//      r7  - Sixth argument to TZ syscall.
//
//============================================================================
tz_fastcall:
    func_prologue()
    orr           w0, w0, #0x200      // val 2 in bits 8-11 means fastcall
    and           w12, w0, #0xff      // last nibble is the arg count
    cmp           w12, #6             // if (args > 6)
    b.gt          seven               // then branch
    smc_call(fastcall)
    func_epilogue()
seven:                                // Anything over 6 arguments is on stack
    cmp           w12, #7
    b.ne          eight
    ldr           w8, [sp, #0x10]
    smc_call(seven)
    func_epilogue()
eight:
    cmp           w12, #8
    b.ne          nine  
    ldp           w8, w9, [sp, #0x10]
    smc_call(eight)
    func_epilogue()
nine:
    cmp           w12, #9
    b.ne          ten  
    ldp           w8,w9, [sp, #0x10]
    ldr           w10, [sp, #0x18]
    smc_call(nine)
    func_epilogue()
ten:
    cmp           w12, #10
    b.ne          tz_fastcall_error 
    ldp           w8, w9, [sp, #0x10]
    ldp           w10, w11, [sp, #0x18]
    smc_call(ten)
    func_epilogue()
tz_fastcall_error:
    mov           x0, #-1             // Too many args
    func_epilogue()

//============================================================================
//
//  int tz_armv8_smc_call(uint32 smc_id, uint32 param_id, uint32 parameters[4], uint32 *trusted_os_id)//
//      r0  - SmcId: Owner, IRQ, Architecture, Command ID
//      r1  - ParamId, lowest nibble contains the number of arguments.
//      r2  - First argument to TZ syscall.
//      r3  - Second argument to TZ syscall.
//      r4  - Third argument to TZ syscall.
//      r5  - Fourth argument to TZ syscall or Indirect Parameter List
//      r6  - Trusted OS ID.
//
//============================================================================
tz_armv8_smc_call:
    func_prologue()
    and     w9, w1, 0xF            // last bit determines number of args
    cmp     w9, #10                // if numargs > 10, return -1
    b.gt    tz_armv8_smc_call_error    
    stp     x19, x20, [sp, #-0x10]!    
    mov     x19, x2                // x19 = &parameters[0]
    mov     x20, x3                // x20 = trusted_os_id
    ldp     w2, w3, [x19], #0x8    // w2 = parameters[0], w5 = parameters[1], x19 = &parameters[2]
    ldp     w4, w5, [x19], #0x4    // w4 = parameters[2], w5 = parameters[3], x19 = &parameters[3]
    smc_call(armv8)
    str     w3, [x19, #-0x4]!      // parameters[2] = w3, x19 = &parameters[2]
    stp     w1, w2, [x19, #-0x8]!  // parameters[0] = w1, parameters[1] = w2, x19 = &parameters[0]
    str     w6, [x20]              // *trusted_os_id = w6
    ldp     x19, x20, [sp], #0x10
    func_epilogue()
tz_armv8_smc_call_error:
    mov     x0, #-1
    func_epilogue()
