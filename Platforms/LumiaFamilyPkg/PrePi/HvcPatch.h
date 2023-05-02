// HvcPatch.c: hypervisor patch that traps SMC call and handles PSCI requests
// Copyright (c) 2020 Sarah Purohit, Bingxing Wang

// Some background story: it seems that Windows build higher than 19041 (not
// including 19041) Messed up the HVC-based PSCI call, although it boots fine on
// Qemu/KVM platform. There are undisclosed platforms also reporting issues with
// HVC-based PSCI call. But on 8992/8994 devices, it completely hangs the boot
// process (ACPI.sys refused to enumerate devices)

// While the root cause is not fully investigated, the patch enables booting
// Windows versions higher than 19041. The patch assumes BSP version 1078, which
// is the last production release and should be present on most firmwares.
// Due to the nature of engineering device, Hapenaro might have multiple
// development firmware around the world. I hope they are compatible with the
// research conducted on prod 8994 BSP. If the patch blows up on Hapenaro, users
// are expected to fix by themselves (prototype users are advanced users, aren't
// they?)

#ifndef __HVCPATCH_H__
#define __HVCPATCH_H__

#if defined(SILICON_PLATFORM)

// Patch location are identical between platforms.
#define WAKE_FROM_POWERGATE_PATCH_ADDR 0x6C001F8
#define LOWER_EL_SYNC_EXC_64B_PATCH_ADDR 0x6C08C24
#define LOWER_EL_SYNC_EXC_32B_PATCH_ADDR 0x6C08E24

// Set HCR_EL2.TSC upon powergate wake-up.
// This patch is shared between platforms.
UINT32 WakeFromPowerGatePatchHandler[] = {
    0xd53c1108, // mrs  x8, HCR_EL2
    0xb26d0108, // orr  x8, x8, #(HCR_EL2.TSC)
    0xd51c1108, // msr  HCR_EL2, x8
    0xd5033fdf, // isb
};

#if SILICON_PLATFORM == 8992
UINT32 LowerELSynchronous64PatchHandler[] = {
    0xd53c1110, // mrs  x16, HCR_EL2
    0xb26d0210, // orr  x16, x16, #(HCR_EL2.TSC)
    0xd51c1110, // msr  HCR_EL2, x16
    0xd5033fdf, // isb
    0xf10059ff, // cmp  x15, #0x16             6'b010110 = HVC 64bit
    0x54ff2a60, // b.eq HvcHandlerEntry
    0xf1005dff, // cmp  x15, #0x17             6'b010111 = SMC 64bit trap
    0x54000040, // b.eq El2TrapSmcHandler
    0x17fff991, // b    OtherExceptionHandler
    // El2TrapSmcHandler:
    0xd53c4030, // mrs  x16, ELR_EL2
    0x91001210, // add  x16, x16, #4
    0xd51c4030, // msr  ELR_EL2, x16
    0x121b6810, // and  w16, w0, #0xffffffe0
    0x32020210, // orr  w16, w16, #0x40000000
    0x52b8800f, // mov  w15, #0xc4000000
    0x6b0f021f, // cmp  w16, w15
    0x54000041, // b.ne El2TrapInvokeSmc
    0x17fff947, // b    HvcHandlerEntry
    // El2TrapInvokeSmc:
    0xa8c143ef, // ldp  x15, x16, [sp], #0x10
    0xd4000003, // smc  #0
    0xd69f03e0, // eret
                // Yay there are two instructions space left
};

UINT32 LowerELSynchronous32PatchHandler[] = {
    0xd53c1110, // mrs  x16, HCR_EL2
    0xb26d0210, // orr  x16, x16, #(HCR_EL2.TSC)
    0xd51c1110, // msr  HCR_EL2, x16
    0xd5033fdf, // isb
    0xf10049ff, // cmp  x15, #0x12           6'b010010 = HVC 32bit
    0x54ff1580, // b.eq HvcHandler32Entry
    0xf1004dff, // cmp  x15, #0x13           6'b010011 = SMC 32bit trap
    0x54000040, // b.eq El2TrapSmcHandler32
    0x17fff911, // b    OtherExceptionHandler
    // El2TrapSmcHandler32:
    0xd53c4030, // mrs  x16, ELR_EL2
    0x91001210, // add  x16, x16, #4
    0xd51c4030, // msr  ELR_EL2, x16
    0x121b6810, // and  w16, w0, #0xffffffe0
    0x32020210, // orr  w16, w16, #0x40000000
    0x52b8800f, // mov  w15, #0xc4000000
    0x6b0f021f, // cmp  w16, w15
    0x54000041, // b.ne El2TrapInvokeSmc32
    0x17fff8a0, // b    OtherExceptionHandler
    // El2TrapInvokeSmc32:
    0xa8c143ef, // ldp  x15, x16, [sp], #0x10
    0xd4000003, // smc  #0
    0xd69f03e0, // eret
                // Yay there are two instructions space left
};
#elif SILICON_PLATFORM == 8994
UINT32 LowerELSynchronous64PatchHandler[] = {
    0xd53c1110, // mrs  x16, HCR_EL2
    0xb26d0210, // orr  x16, x16, #(HCR_EL2.TSC)
    0xd51c1110, // msr  HCR_EL2, x16
    0xd5033fdf, // isb
    0xf10059ff, // cmp  x15, #0x16             6'b010110 = HVC 64bit
    0x54ff4660, // b.eq HvcHandlerEntry
    0xf1005dff, // cmp  x15, #0x17             6'b010111 = SMC 64bit trap
    0x54000040, // b.eq El2TrapSmcHandler
    0x17fffa71, // b    OtherExceptionHandler
    // El2TrapSmcHandler:
    0xd53c4030, // mrs  x16, ELR_EL2
    0x91001210, // add  x16, x16, #4
    0xd51c4030, // msr  ELR_EL2, x16
    0x121b6810, // and  w16, w0, #0xffffffe0
    0x32020210, // orr  w16, w16, #0x40000000
    0x52b8800f, // mov  w15, #0xc4000000
    0x6b0f021f, // cmp  w16, w15
    0x54000041, // b.ne El2TrapInvokeSmc
    0x17fffa27, // b    HvcHandlerEntry
    // El2TrapInvokeSmc:
    0xa8c143ef, // ldp  x15, x16, [sp], #0x10
    0xd4000003, // smc  #0
    0xd69f03e0, // eret
                // Yay there are two instructions space left
};

UINT32 LowerELSynchronous32PatchHandler[] = {
    0xd53c1110, // mrs  x16, HCR_EL2
    0xb26d0210, // orr  x16, x16, #(HCR_EL2.TSC)
    0xd51c1110, // msr  HCR_EL2, x16
    0xd5033fdf, // isb
    0xf10049ff, // cmp  x15, #0x12           6'b010010 = HVC 32bit
    0x54ff3180, // b.eq HvcHandler32Entry
    0xf1004dff, // cmp  x15, #0x13           6'b010011 = SMC 32bit trap
    0x54000040, // b.eq El2TrapSmcHandler32
    0x17fff9f1, // b    OtherExceptionHandler
    // El2TrapSmcHandler32:
    0xd53c4030, // mrs  x16, ELR_EL2
    0x91001210, // add  x16, x16, #4
    0xd51c4030, // msr  ELR_EL2, x16
    0x121b6810, // and  w16, w0, #0xffffffe0
    0x32020210, // orr  w16, w16, #0x40000000
    0x52b8800f, // mov  w15, #0xc4000000
    0x6b0f021f, // cmp  w16, w15
    0x54000041, // b.ne El2TrapInvokeSmc32
    0x17fff980, // b    OtherExceptionHandler
    // El2TrapInvokeSmc32:
    0xa8c143ef, // ldp  x15, x16, [sp], #0x10
    0xd4000003, // smc  #0
    0xd69f03e0, // eret
                // Yay there are two instructions space left
};
#else
#error Unsupported silicon platform
#endif // SILICON_PLATFORM

#else
#error Undefined silicon platform
#endif // defined(SILICON_PLATFORM)

#endif // __HVCPATCH_H__
