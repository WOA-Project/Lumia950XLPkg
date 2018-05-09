#include <Base.h>

#include <Library/LKEnvLib.h>
#include <Library/QcomSmemLib.h>

#include "smem_p.h"

QCOM_SMEM_PROTOCOL *gSMEM = NULL;

STATIC QCOM_SMEM_PROTOCOL mInternalSMEM = {
  smem_read_alloc_entry,
  smem_read_alloc_entry_offset,
  smem_get_alloc_entry,
};

RETURN_STATUS
EFIAPI
SmemImplLibInitialize (
  VOID
  )
{
  gSMEM = &mInternalSMEM;
  return RETURN_SUCCESS;
}
