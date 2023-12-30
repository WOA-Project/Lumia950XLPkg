#include <Base.h>

#include <Library/LKEnvLib.h>
#include <Library/QcomSmemPtableLib.h>

#include "smem_ptable_p.h"

QCOM_SMEM_PTABLE_PROTOCOL *gSmemPtable = NULL;

STATIC QCOM_SMEM_PTABLE_PROTOCOL mInternalSmemPtable = {
    smem_get_apps_flash_start, smem_add_modem_partitions,

    smem_get_ram_ptable_entry, smem_get_ram_ptable_version,
    smem_get_ram_ptable_len,   get_ddr_start,
};

RETURN_STATUS
EFIAPI
SmemPtableImplLibInitialize(VOID)
{
  gSmemPtable = &mInternalSmemPtable;

  smem_ptable_init();
  smem_ram_ptable_init_v1();

  return RETURN_SUCCESS;
}
