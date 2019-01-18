// PlatformInfo Shim
// This is intended to maintain compatibility with Qualcomm binary module.
// Do not invoke if not necessary.

#ifndef __PLATFORM_INFO_SHIM_H__
#define __PLATFORM_INFO_SHIM_H__

#define EFI_PLATFORMINFO_SHIM_PROTOCOL_VERSION 0x0000000000020000
#define EFI_PLATFORMINFO_SHIM_PROTOCOL_GUID \
  { 0x157a5c45, 0x21b2 , 0x43c5, { 0xba, 0x7c, 0x82, 0x2f, 0xee, 0x5f, 0xe5, 0x99 } }

extern EFI_GUID gEfiPlatformInfoShimProtocolGuid;

typedef struct _EFI_PLATFORMINFO_SHIM_PROTOCOL EFI_PLATFORMINFO_SHIM_PROTOCOL;

typedef enum
{
	EFI_PLATFORMINFO_TYPE_UNKNOWN = 0x00,  /**< Unknown target device. */
	EFI_PLATFORMINFO_TYPE_SURF = 0x01,  /**< Subscriber unit reference device. */
	EFI_PLATFORMINFO_TYPE_FFA = 0x02,  /**< Form-fit accurate device. */
	EFI_PLATFORMINFO_TYPE_FLUID = 0x03,  /**< Forward looking user interface
										 demonstration device. */
	EFI_PLATFORMINFO_TYPE_FUSION = 0x04,  /**< Fusion device. */
	EFI_PLATFORMINFO_TYPE_OEM = 0x05,  /**< Original equipment manufacturer
									   device. */
	EFI_PLATFORMINFO_TYPE_QT = 0x06,  /**< Qualcomm tablet device. */
	EFI_PLATFORMINFO_TYPE_CDP = EFI_PLATFORMINFO_TYPE_SURF,
	/**< CDP (a.k.a. SURF) device. */
	EFI_PLATFORMINFO_TYPE_MTP_MDM = 0x07,  /**< MDM MTP device. */
	EFI_PLATFORMINFO_TYPE_MTP_MSM = 0x08,  /**< MSM@tm MTP device. */
	EFI_PLATFORMINFO_TYPE_MTP = EFI_PLATFORMINFO_TYPE_MTP_MSM,
	EFI_PLATFORMINFO_TYPE_LIQUID = 0x09,  /**< LiQUID device. */
	EFI_PLATFORMINFO_TYPE_DRAGONBOARD = 0x0A,  /**< DragonBoard@tm device. */
	EFI_PLATFORMINFO_TYPE_QRD = 0x0B,  /**< QRD device. */
	EFI_PLATFORMINFO_TYPE_EVB = 0x0C,  /**< EVB device. */
	EFI_PLATFORMINFO_TYPE_HRD = 0x0D,  /**< HRD device. */
	EFI_PLATFORMINFO_TYPE_DTV = 0x0E,  /**< DTV device. */
	EFI_PLATFORMINFO_TYPE_RUMI = 0x0F,  /**< Target is on Rumi (ASIC emulation). */
	EFI_PLATFORMINFO_TYPE_VIRTIO = 0x10,  /**< Target is on Virtio (system-level simulation). */
	EFI_PLATFORMINFO_TYPE_GOBI = 0x11,  /**< Gobi@tm device. */
	EFI_PLATFORMINFO_TYPE_CBH = 0x12,  /**< CBH device. */
	EFI_PLATFORMINFO_TYPE_BTS = 0x13,  /**< BTS device. */
	EFI_PLATFORMINFO_TYPE_XPM = 0x14,  /**< XPM device. */
	EFI_PLATFORMINFO_TYPE_RCM = 0x15,  /**< RCM device. */
	EFI_PLATFORMINFO_TYPE_DMA = 0x16,  /**< DMA device. */
	EFI_PLATFORMINFO_TYPE_STP = 0x17,  /**< STP device. */
	EFI_PLATFORMINFO_TYPE_SBC = 0x18,  /**< SBC device. */
	EFI_PLATFORMINFO_TYPE_ADP = 0x19,  /**< ADP device. */
	EFI_PLATFORMINFO_TYPE_CHI = 0x1A,  /**< CHI device. */
	EFI_PLATFORMINFO_NUM_TYPES,
	EFI_PLATFORMINFO_TYPE_32BITS = 0x7FFFFFFF
} EFI_PLATFORMINFO_PLATFORM_TYPE;

/**
Stores the target platform, the platform version, and the subtype of the
platform.
*/
typedef struct
{
	EFI_PLATFORMINFO_PLATFORM_TYPE    platform;
	/**< Type of the current target; see #EFI_PLATFORMINFO_PLATFORM_TYPE for
	details. */
	UINT32                            version;
	/**< Version of the platform in use. */
	UINT32                            subtype;
	/**< Subtype of the platform. */
	BOOLEAN                           fusion;
	/**< TRUE if Fusion; FALSE otherwise. */
} EFI_PLATFORMINFO_PLATFORM_INFO_TYPE;

typedef
EFI_STATUS
(EFIAPI *EFI_PLATFORMINFO_GET_PLATFORMINFO)(
	IN  EFI_PLATFORMINFO_SHIM_PROTOCOL           *This,
	OUT EFI_PLATFORMINFO_PLATFORM_INFO_TYPE		 *PlatformInfo
);

struct _EFI_PLATFORMINFO_SHIM_PROTOCOL {
	UINT64                            Version;
	EFI_PLATFORMINFO_GET_PLATFORMINFO GetPlatformInfo;
	void*							  NotSupported;
};

#endif