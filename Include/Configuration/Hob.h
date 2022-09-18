#ifndef __LOCAL_HOB_H__
#define __LOCAL_HOB_H__

#define PRELOADER_ENV_ADDR 0xb0000000
#define PRELOADER_VERSION_MIN 0x2000

#define PRELOADER_HEADER SIGNATURE_32('B', 'S', 'E', 'N')

typedef enum _PRELOADER_ENVIRONMENT_BOOT_MODE {
	BOOT_MODE_PSCI = 0,
	BOOT_MODE_MPPARK,
	BOOT_MODE_MPPARK_EL2,
	BOOT_MODE_MAX
} PRELOADER_ENVIRONMENT_BOOT_MODE;

typedef struct _PRELOADER_ENVIRONMENT_VERSION_1 {
  UINT32   Header;
  UINT32   PreloaderVersion;
  CHAR8    PreloaderRelease[64];
  EFI_TIME BootTimeEpoch;
  UINT32   UefiDisplayInfo[30];
  UINT32   Crc32;
} PRELOADER_ENVIRONMENT_VERSION_1, * PPRELOADER_ENVIRONMENT_VERSION_1;

typedef struct _PRELOADER_ENVIRONMENT_VERSION_2 {
  UINT32   Header;
  UINT32   PreloaderVersion;
  CHAR8    PreloaderRelease[64];
  EFI_TIME BootTimeEpoch;
  UINT32   UefiDisplayInfo[30];
  UINT32   Crc32;
  UINT32   BootMode;
  UINT32   EnablePlatformSdCardBoot;
  UINT32   UseQuadCoreConfiguration;
  UINT32   Crc32v2;
} PRELOADER_ENVIRONMENT_VERSION_2, *PPRELOADER_ENVIRONMENT_VERSION_2;

#endif