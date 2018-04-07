#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/QcomBam.h>

#include "bam_p.h"

STATIC QCOM_BAM_PROTOCOL mBam = {
  bam_init,
  bam_sys_pipe_init,
  bam_pipe_fifo_init,
  bam_add_cmd_element,
  bam_add_desc,
  bam_add_one_desc,
  bam_sys_gen_event,
  bam_wait_for_interrupt,
  bam_read_offset_update,
};

EFI_STATUS
EFIAPI
BamDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_HANDLE Handle = NULL;
  EFI_STATUS Status;

  Status = gBS->InstallMultipleProtocolInterfaces(
                  &Handle,
                  &gQcomBamProtocolGuid,      
                  &mBam,
                  NULL
                  );
  ASSERT_EFI_ERROR(Status);

  return Status;
}
