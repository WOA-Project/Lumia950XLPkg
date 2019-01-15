#include <PiPei.h>

#include <Library/LKEnvLib.h>
#include <Library/UEFIConfigSh.h>

#define CURRENT_UEFI_CFG_LIB_VERSION  0x00010002

MemRegionInfo m_PCIeWrapperAxi = {
	"PCIE WRAPPER AXI",
	0xF8800000,
	0x00800000,
	AddPeripheral,
	EFI_RESOURCE_MEMORY_MAPPED_IO,
	EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE,
	EfiMemoryMappedIO,
	ARM_MEMORY_REGION_ATTRIBUTE_DEVICE
};

MemRegionInfo m_PCIeWrapperAhb = {
	"PCIE WRAPPER AHB",
	0xFC528000,
	0x00800000,
	AddPeripheral,
	EFI_RESOURCE_MEMORY_MAPPED_IO,
	EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE,
	EfiMemoryMappedIO,
	ARM_MEMORY_REGION_ATTRIBUTE_DEVICE
};

EFI_STATUS EFIAPI
GetMemRegionInfoByName(
	CHAR8* RegionName,
	MemRegionInfo* MemoryRegion
)
{
	if (AsciiStrCmp(RegionName, "PCIE WRAPPER AXI") == 0)
	{
		MemoryRegion = &m_PCIeWrapperAxi;
		return EFI_SUCCESS;
	}
	else if (AsciiStrCmp(RegionName, "PCIE WRAPPER AHB") == 0)
	{
		MemoryRegion = &m_PCIeWrapperAhb;
		return EFI_SUCCESS;
	}

	return EFI_NOT_FOUND;
}

EFI_STATUS
GetConfigStr(
	CHAR8* Key, 
	CHAR8* Value, 
	UINTN* ValBuffSize
)
{
	return EFI_UNSUPPORTED;
}

EFI_STATUS
GetConfigVal(
	CHAR8* Key, 
	UINT32* Value
)
{
	if (AsciiStrCmp(Key, "PCIeRPNumber") == 0)
	{
		*Value = 1;
		return EFI_SUCCESS;
	}
	else if (AsciiStrCmp(Key, "SharedIMEMBaseAddr") == 0)
	{
		*Value = 0xFE87F000;
		return EFI_SUCCESS;
	}

	return EFI_NOT_FOUND;
}

EFI_STATUS
GetConfigVal64(
	CHAR8* Key, 
	UINT64* Value
)
{
	if (AsciiStrCmp(Key, "PCIeRPNumber") == 0)
	{
		*Value = 1;
		return EFI_SUCCESS;
	}
	else if (AsciiStrCmp(Key, "SharedIMEMBaseAddr") == 0)
	{
		*Value = 0xFE87F000;
		return EFI_SUCCESS;
	}

	return EFI_NOT_FOUND;
}

UefiCfgLibType UefiCfgLib =
{
  .LibVersion = CURRENT_UEFI_CFG_LIB_VERSION,
  .GetMemInfoByName = GetMemRegionInfoByName,
  .GetCfgInfoString = GetConfigStr,
  .GetCfgInfoVal = GetConfigVal,
  .GetCfgInfoVal64 = GetConfigVal64
};
