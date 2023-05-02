#include <Base.h>

#include <Library/LKEnvLib.h>

#include <Library/PlatformIdLib.h>
#include <Library/QcomBoardLib.h>

#include <Chipset/smem.h>

BOOLEAN platform_is_8974(VOID)
{
  BOOLEAN ret;
  UINT32  platform = gBoard->board_platform_id();

  switch (platform) {
  case APQ8074:
  case MSM8274:
  case MSM8674:
  case MSM8974:
    ret = TRUE;
    break;

  default:
    ret = 0;
    break;
  }

  return ret;
}

BOOLEAN platform_is_8974Pro(VOID)
{
  BOOLEAN ret;
  UINT32  platform = gBoard->board_platform_id();

  switch (platform) {
  case MSM8974AC:
  case MSM8674AC:
  case MSM8274AC:
  case APQ8074AC:
    ret = TRUE;
    break;
  default:
    ret = FALSE;
    break;
  }

  return ret;
}

BOOLEAN platform_is_msm8994(VOID)
{
  UINT32 platform = gBoard->board_platform_id();
  if ((platform == APQ8094) || (platform == MSM8994))
    return TRUE;
  else
    return FALSE;
}

BOOLEAN platform_is_msm8992(VOID)
{
  BOOLEAN ret;
  UINT32  platform = gBoard->board_platform_id();

  switch (platform) {
  case MSM8992:
  case APQ8092:
    ret = TRUE;
    break;

  default:
    ret = FALSE;
    break;
  }

  return ret;
}
