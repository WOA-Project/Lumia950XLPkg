#include <Base.h>

#include <Library/LKEnvLib.h>
#include <Library/QcomSpmiLib.h>

#include "spmi_p.h"

QCOM_SPMI_PROTOCOL *gSpmi = NULL;

STATIC QCOM_SPMI_PROTOCOL mInternalSpmi = {
  pmic_arb_write_cmd,
  pmic_arb_read_cmd,
};

RETURN_STATUS
EFIAPI
SpmiImplLibInitialize (
  VOID
  )
{
  gSpmi = &mInternalSpmi;

  int rc = spmi_init();
  if (rc) {
    return RETURN_DEVICE_ERROR;
  }

  return RETURN_SUCCESS;
}
