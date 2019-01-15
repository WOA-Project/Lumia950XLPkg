#include <PiPei.h>

#include <Library/LKEnvLib.h>
#include <Library/SharedLib.h>
#include <Library/SharedLibInstall.h>
#include <Library/SerialPortSh.h>
#include <Library/UEFIConfigSh.h>

#include <Pi/PiHob.h>
#include <Library/HobLib.h>

#define SH_LIB_LOADER_VERSION		0x00010001
#define GET_MAJOR_VER(x)			(((UINT32)(x)) >> 16)
#define MAX_LIB_COUNT				(32U)
#define MAX_NAME_BUFFER_SIZE		(MAX_LIB_COUNT * 32)

extern SioPortLibType SioLib;
extern UefiCfgLibType UefiCfgLib;

SioPortLibType* m_sioLib;
UefiCfgLibType* m_cfgLib;

EFI_STATUS InstallLibrary(
	CHAR8* LibName, 
	UINT32 LibVersion, 
	VOID *LibIntf
);

EFI_STATUS LoadLibrary(
	CHAR8* LibName, 
	UINT32 LibVersion, 
	VOID **LibIntfPtr
);

ShLibLoaderType ShLibMgr =
{
	SH_LIB_LOADER_VERSION,
	InstallLibrary,
	LoadLibrary,
};

typedef struct {
	CHAR8*      Name;
	UINT32      Version;
	VOID*       Intf;
} ShLibModuleType;

typedef struct {
	UINT32            LoaderVersion;
	ShLibModuleType   Lib[MAX_LIB_COUNT];
	UINT32            LoadedLibCount;
	CHAR8*            NameBufferPtr;
	CHAR8*            NameBufferEnd;
	CHAR8             NameBuffer[MAX_NAME_BUFFER_SIZE];
} ShLibMgrType;

ShLibMgrType ShLib;

extern EFI_GUID gShimSharedLibHobGuid;

STATIC 
EFI_STATUS
InstallShLibLoader(
	ShLibLoaderType* Intf
)
{
	UINTN Data = (UINTN) Intf;
	BuildGuidDataHob(
		&gShimSharedLibHobGuid, 
		&Data, 
		sizeof(Data)
	);

	return 0;
}

EFI_STATUS
InstallLibrary(
	CHAR8* LibName,
	UINT32 LibVersion,
	VOID *LibIntf
)
{
	// Not really going to install the library.
	// The only known library that will be invoked is serial IO library

	return EFI_SUCCESS;
}

EFI_STATUS
LoadLibrary(
	CHAR8* LibName, 
	UINT32 LibVersion, 
	VOID **LibIntfPtr
)
{
	// Whatever invoked will have either UEFI config library
	// or serial IO port library returned
	if (AsciiStriCmp(LibName, SIO_PORT_LIB_NAME) == 0)
	{
		*LibIntfPtr = m_sioLib;
		return LIB_SUCCESS;
	}
	else if (AsciiStriCmp(LibName, UEFI_CFG_LIB_NAME) == 0)
	{
		*LibIntfPtr = m_cfgLib;
		return LIB_SUCCESS;
	}

	return LIB_ERR_NOT_FOUND;
}

EFI_STATUS
ShLibMgrPeim(
	VOID
)
{
	m_sioLib = &SioLib;
	m_cfgLib = &UefiCfgLib;

	SetMem(&ShLib, sizeof(ShLib), 0);
	ShLib.LoaderVersion = SH_LIB_LOADER_VERSION;
	ShLib.NameBufferPtr = ShLib.NameBuffer;
	ShLib.NameBufferEnd = ShLib.NameBuffer + sizeof(ShLib.NameBuffer);
	return InstallShLibLoader(&ShLibMgr);
}
