#ifndef __SHARED_LIB_H__
#define __SHARED_LIB_H__

#include <Uefi.h>

#define  LIB_SUCCESS                        (EFI_SUCCESS)
#define  LIB_CLOSE_MATCH_FOUND              (EFI_NO_MAPPING)

#define  LIB_ERR_NOT_FOUND                  (EFI_NOT_FOUND)
#define  LIB_ERR_INVALID_PARAM              (EFI_INVALID_PARAMETER)
#define  LIB_ERR_ALREADY_INSTALLED          (EFI_ALREADY_STARTED)
#define  LIB_ERR_OUT_OF_RESOURCES           (EFI_OUT_OF_RESOURCES)
#define  LIB_ERR_INCOMPATIBLE_LOADER        (EFI_UNSUPPORTED)

#define  LIB_MAJOR_VERSION_NUMBER_MASK      (0xFFFF0000U)

typedef EFI_STATUS(*INSTALL_LIB) (
	IN  CHAR8   *LibName,
	IN  UINT32   LibVersion,
	IN  VOID    *LibIntf
);

typedef EFI_STATUS(*LOAD_LIB) (
	IN  CHAR8     *LibName,
	IN  UINT32     LibVersion,
	OUT VOID     **LibIntfPtr
);

typedef struct {
	UINT32          LoaderVersion;
	INSTALL_LIB     InstallLib;
	LOAD_LIB        LoadLib;
} ShLibLoaderType;

#endif