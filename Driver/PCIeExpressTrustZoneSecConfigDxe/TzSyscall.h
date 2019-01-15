#ifndef TZ_SYSCALL_H
#define TZ_SYSCALL_H

/**
@file tzt_syscall.h
@brief Main head for all system calls

*/
/*===========================================================================
Copyright (c) 2012, 2014 Copyright Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

							EDIT HISTORY FOR FILE


when       who      what, where, why
--------   ---      ------------------------------------
07/09/14   sm       Removed unused TZT remnants. Added SMCv8.
12/01/12   shl      Renamed and reorged.
08/30/10   cap      Initial version.

===========================================================================*/

#define MAX_LISTENER_NUM     10

EFI_STATUS
ScmListenerLookUpIndex(
	IN  UINT32     ListenerId,
	OUT UINT32    *Index
);


/*----------------------------------------------------------------------------
 * Common
 * -------------------------------------------------------------------------*/
 // TZ Return Statuses
#define TZ_RETURN_SUCCESS               1 // Success
#define TZ_RETURN_FAIL                 -1 // failed due to unknown error
#define TZ_RETURN_OPERATION_NOT_VALID  -4 // Operation not yet implemented
#define TZ_RETURN_ALREADY_DONE        -13 // Operation already done


/*----------------------------------------------------------------------------
 * ARMv7 Calls
 * -------------------------------------------------------------------------*/

 // TZ memory protection requirement
#define TZ_MEM_ALIGNMENT_SIZE       4096

extern int tz_slowcall(const void * cmd_addr);
extern int tz_fastcall(UINT32 id, UINT32 token, UINT32 r0, UINT32 r1, ...);
extern int tz_get_version(UINT32* version);

/*----------------------------------------------------------------------------
 * ARMv8 Calls
 * -------------------------------------------------------------------------*/

typedef enum
{
	SMC_INTERRUPTED = 1,  /* SMC call was interrupted                */
	SMC_SUCCESS = 0,  /* Success, requested syscall was called   */
	SMC_ERR_UNKNOWN_SMC_ID = -1,  /* ARM-defined error of 0xFFFFFFFF         */
	SMC_ERR_SYSCALL_FAILED = -2,  /* Syscall function returned failure       */
	SMC_ERR_SYSCALL_NOT_AVAILABLE = -3,  /* No function associated with syscall def */
	SMC_ERR_RESERVED_FIELD_MBZ = -4,  /* SMC ID err: reserved field must be zero */
	SMC_ERR_NUM_ARGS_MISMATCH = -5,  /* Num args does not match func definition */
	SMC_ERR_INDIRECT_PARAM_PTR_NOT_NS = -6,  /* Ind. param ptr doesnt point to ns mem   */
	SMC_ERR_BUF_LEN_NOT_FOUND = -7,  /* No buffer len following buffer ptr      */
	SMC_ERR_BUF_PTR_NOT_NS = -8,  /* Buffer ptr does not point to ns mem     */
	SMC_ERR_NO_MEMORY = -9,  /* Heap is out of space                    */
	SMC_ERR_PARAM_ID_MISMATCH = -10, /* Incorrect Param ID from NS world        */
	SMC_ERR_YOU_BROKE_IT = -11, /* How could you?  (unused) */
	SMC_ERR_BUSY = -12, /* TZ is busy waiting for listener rsp     */

	/* ARMv8 SMC spec: Must return 0xFFFFFFFF for following errors */
	SMC_ERR_SYSCALL_NOT_SUPPORTED = SMC_ERR_UNKNOWN_SMC_ID,
	SMC_ERR_AARCH64_NOT_SUPPORTED = SMC_ERR_UNKNOWN_SMC_ID,
} SmcErrnoType;

#define SMC_PARAM_TYPE_VALUE              0
#define SMC_PARAM_TYPE_BUFFER_READ        1
#define SMC_PARAM_TYPE_BUFFER_READWRITE   2
#define SMC_PARAM_TYPE_BUFFER_VALIDATION  3

#define SMC_AARCH32 0
#define SMC_AARCH64 1

#define SMC_IRQS_DISABLED 1
#define SMC_IRQS_ENABLED  0

#define NUM_DIRECT_REQUEST_PARAMETERS 4

#define TZ_CREATE_SMC_ID(irq, aarch, owner, id) \
  (((irq & 0x1U) << 31) | ((aarch & 0x1) << 30) | ((owner & 0x3f) << 24) | (id & 0xffff))

#define TZ_CREATE_SMC_ID_WITH_RESERVED_FIELD(irq, aarch, owner, rsvd, id) \
  (((irq & 0x1U) << 31) | ((aarch & 0x1) << 30) | ((owner & 0x3f) << 24) | ((rsvd & 0xff) << 16) | (id & 0xffff))

#define TZ_FAST_SMC_ID(id) \
  (((SMC_IRQS_DISABLED & 0x1U) << 31) | ((SMC_AARCH32 & 0x1) << 30) | ((TZ_SIP_SYSCALL & 0x3f) << 24) | (id & 0xffff))

extern SmcErrnoType tz_armv8_smc_call(UINT32 smc_id, UINT32 param_id, UINT32 parameters[4], UINT32 *trusted_os_id);

#endif /* TZ_SYSCALL_H */
