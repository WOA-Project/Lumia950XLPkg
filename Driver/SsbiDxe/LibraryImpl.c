#include <Base.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomSsbiLib.h>

#include "ssbi.h"

QCOM_SSBI_PROTOCOL *gSSBI = NULL;

STATIC QCOM_SSBI_PROTOCOL mInternalSSBI = {
    i2c_ssbi_read_bytes,   i2c_ssbi_write_bytes, pa1_ssbi2_read_bytes,
    pa1_ssbi2_write_bytes, pa2_ssbi2_read_bytes, pa2_ssbi2_write_bytes,
};

RETURN_STATUS
EFIAPI
SsbiImplLibInitialize(VOID)
{
  gSSBI = &mInternalSSBI;
  return RETURN_SUCCESS;
}
