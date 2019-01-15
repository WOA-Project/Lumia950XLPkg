#ifndef __PCIE_TRUSTZONE_H__
#define __PCIE_TRUSTZONE_H__

/**
  * Memory protection-related device types.
  */
typedef enum
{
	TZ_DEVICE_VIDEO = 0,          /**< Video subsystem. */
	TZ_DEVICE_MDSS = 1,           /**< MDSS subsystem. */
	TZ_DEVICE_LPASS = 2,          /**< Low-power audio subsystem. */
	TZ_DEVICE_MDSS_BOOT = 3,      /**< MDSS subsystem at cold boot. */
	TZ_DEVICE_USB1_HS = 4,        /**< High speed USB. */
	TZ_DEVICE_OCMEM = 5,          /**< OCMEM registers. @newpage */
	TZ_DEVICE_LPASS_CORE = 6,
	TZ_DEVICE_VPU = 7,
	TZ_DEVICE_COPSS_SMMU = 8,
	TZ_DEVICE_USB3_0 = 9,
	TZ_DEVICE_USB3_1 = 10,
	TZ_DEVICE_PCIE_0 = 11,
	TZ_DEVICE_PCIE_1 = 12,
	TZ_DEVICE_BCSS = 13,
	TZ_DEVICE_VCAP = 14,
	TZ_DEVICE_PCIE20 = 15,
	TZ_DEVICE_IPA = 16,
	TZ_DEVICE_APPS = 17,
	TZ_DEVICE_GPU = 18,
	TZ_DEVICE_COUNT,
	TZ_DEVICE_MAX = 0x7FFFFFFF,
} tz_device_e_type;

typedef struct tz_restore_sec_cfg_req_s
{
	UINT32                        device;
	/**< Device to be restored;
		 see #tz_device_e_type. */
	UINT32                        spare;       /**< Spare. */
} __attribute__((packed)) tz_restore_sec_cfg_req_t;

/**
   TODO Remove once legacy interface is removed
   Common system call response header used in all system calls that
   have responses.
 */
typedef struct tz_syscall_rsp_s
{
	UINT32  len;                 /**< Total length of the response. */

	UINT32  response_start_off;  /**< Start offset of the memory to where
									  the response information is to be
									  written. The offset is computed
									  from the start of this
									  structure. */

	INT32   status;              /**< Status of the syscall_req
									  executed: \n
									  0 means the command was not completed \n
									  1 means the command was completed */
} __attribute__((packed)) tz_syscall_rsp_t;

#define SCM_MAX_NUM_PARAMETERS           10
#define SCM_MAX_NUM_RESULTS              4

/**
   Macro used to define an SMC ID based on the owner ID,
   service ID, and function number.
*/
#define TZ_SYSCALL_CREATE_SMC_ID(o, s, f) \
  ((UINT32)((((o & 0x3f) << 24 ) | (s & 0xff) << 8) | (f & 0xff)))

/** SIP service call ID */
#define TZ_OWNER_SIP                     2
#define TZ_SVC_MEMORY_PROTECTION  12      /* Memory protection service.     */

/** General helper macro to create a bitmask from bits low to high. */
#define TZ_MASK_BITS(h,l)     ((0xffffffff >> (32 - ((h - l) + 1))) << l)

#define TZ_SYSCALL_PARAM_NARGS_MASK  TZ_MASK_BITS(3,0)
#define TZ_SYSCALL_PARAM_TYPE_MASK   TZ_MASK_BITS(1,0)

#define TZ_SYSCALL_CREATE_PARAM_ID(nargs, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10) \
  ((nargs&TZ_SYSCALL_PARAM_NARGS_MASK)+ \
  ((p1&TZ_SYSCALL_PARAM_TYPE_MASK)<<4)+ \
  ((p2&TZ_SYSCALL_PARAM_TYPE_MASK)<<6)+ \
  ((p3&TZ_SYSCALL_PARAM_TYPE_MASK)<<8)+ \
  ((p4&TZ_SYSCALL_PARAM_TYPE_MASK)<<10)+ \
  ((p5&TZ_SYSCALL_PARAM_TYPE_MASK)<<12)+ \
  ((p6&TZ_SYSCALL_PARAM_TYPE_MASK)<<14)+ \
  ((p7&TZ_SYSCALL_PARAM_TYPE_MASK)<<16)+ \
  ((p8&TZ_SYSCALL_PARAM_TYPE_MASK)<<18)+ \
  ((p9&TZ_SYSCALL_PARAM_TYPE_MASK)<<20)+ \
  ((p10&TZ_SYSCALL_PARAM_TYPE_MASK)<<22))

/**
   Macros used to create the Parameter ID associated with the syscall
 */
#define TZ_SYSCALL_CREATE_PARAM_ID_0 0
#define TZ_SYSCALL_CREATE_PARAM_ID_1(p1) \
  TZ_SYSCALL_CREATE_PARAM_ID(1, p1, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define TZ_SYSCALL_CREATE_PARAM_ID_2(p1, p2) \
  TZ_SYSCALL_CREATE_PARAM_ID(2, p1, p2, 0, 0, 0, 0, 0, 0, 0, 0)
#define TZ_SYSCALL_CREATE_PARAM_ID_3(p1, p2, p3) \
  TZ_SYSCALL_CREATE_PARAM_ID(3, p1, p2, p3, 0, 0, 0, 0, 0, 0, 0)
#define TZ_SYSCALL_CREATE_PARAM_ID_4(p1, p2, p3, p4) \
  TZ_SYSCALL_CREATE_PARAM_ID(4, p1, p2, p3, p4, 0, 0, 0, 0, 0, 0)
#define TZ_SYSCALL_CREATE_PARAM_ID_5(p1, p2, p3, p4, p5) \
  TZ_SYSCALL_CREATE_PARAM_ID(5, p1, p2, p3, p4, p5, 0, 0, 0, 0, 0)
#define TZ_SYSCALL_CREATE_PARAM_ID_6(p1, p2, p3, p4, p5, p6) \
  TZ_SYSCALL_CREATE_PARAM_ID(6, p1, p2, p3, p4, p5, p6, 0, 0, 0, 0)
#define TZ_SYSCALL_CREATE_PARAM_ID_7(p1, p2, p3, p4, p5, p6, p7) \
  TZ_SYSCALL_CREATE_PARAM_ID(7, p1, p2, p3, p4, p5, p6, p7, 0, 0, 0)
#define TZ_SYSCALL_CREATE_PARAM_ID_8(p1, p2, p3, p4, p5, p6, p7, p8) \
  TZ_SYSCALL_CREATE_PARAM_ID(8, p1, p2, p3, p4, p5, p6, p7, p8, 0, 0)
#define TZ_SYSCALL_CREATE_PARAM_ID_9(p1, p2, p3, p4, p5, p6, p7, p8, p9) \
  TZ_SYSCALL_CREATE_PARAM_ID(9, p1, p2, p3, p4, p5, p6, p7, p8, p9, 0)
#define TZ_SYSCALL_CREATE_PARAM_ID_10(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10) \
  TZ_SYSCALL_CREATE_PARAM_ID(10, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)

 /** A parameter of type value */
#define TZ_SYSCALL_PARAM_TYPE_VAL              0x0
/** A parameter of type buffer read-only */
#define TZ_SYSCALL_PARAM_TYPE_BUF_RO           0x1
/** A parameter of type buffer read-write */
#define TZ_SYSCALL_PARAM_TYPE_BUF_RW           0x2

#define TZ_RESTORE_SEC_CFG  TZ_SYSCALL_CREATE_SMC_ID(TZ_OWNER_SIP, TZ_SVC_MEMORY_PROTECTION, 0x02)

#define TZ_RESTORE_SEC_CFG_PARAM_ID \
  TZ_SYSCALL_CREATE_PARAM_ID_2( \
      TZ_SYSCALL_PARAM_TYPE_VAL, TZ_SYSCALL_PARAM_TYPE_VAL )

#endif
