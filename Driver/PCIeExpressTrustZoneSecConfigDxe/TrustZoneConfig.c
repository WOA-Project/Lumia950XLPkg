#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <IndustryStandard/Pci22.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/ArmSmcLib.h>
#include "TrustZone.h"
#include "TzSyscall.h"

EFI_STATUS
EFIAPI
PCIeExpressTrustZoneSecConfigDxeEntry(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	UINT64 Parameters[SCM_MAX_NUM_PARAMETERS] = { 0 };
	UINT32 InternalTrustedOsId = 0;
	UINT32 DirectParameters[NUM_DIRECT_REQUEST_PARAMETERS] = { 0 };
	SmcErrnoType TzStatus = SMC_SUCCESS;
	tz_restore_sec_cfg_req_t *pSysCallReq = (tz_restore_sec_cfg_req_t*) Parameters;

	// Initializing cmd structure for scm sys call
	pSysCallReq->device = TZ_DEVICE_PCIE_1;
	pSysCallReq->spare = 0;

	DirectParameters[0] = Parameters[0];
	DirectParameters[1] = Parameters[1];

	TzStatus = tz_armv8_smc_call(
		TZ_RESTORE_SEC_CFG,
		TZ_RESTORE_SEC_CFG_PARAM_ID,
		DirectParameters,
		&InternalTrustedOsId
	);

	ASSERT(TzStatus == SMC_SUCCESS);
	return EFI_SUCCESS;
}
