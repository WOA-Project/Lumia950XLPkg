#include <PiDxe.h>

#include <Library/DxeServicesTableLib.h>
#include <Library/LKEnvLib.h>
#include <Library/RuntimeSpmiMemoryConfig.h>

#include <Platform/iomap.h>

#define SPMI_BASE ((UINTN)PcdGet64(PcdSpmiBaseAddress))

EFI_STATUS
EFIAPI
RtSpmiConfigMemory(VOID)
{
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR psHoldMemoryDescriptor;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR imemRegionMemoryDescriptor;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR spmiMemoryDescriptor;

  EFI_STATUS Status;

  // Ideally, we should run this code in ResetLibrary, but it is
  // impossible for some deps reasons. So we made a library, and
  // let a DXE service to run it.

  // Register memory regions

  // Mark PSHold (0xFC4A0000 start) / IMEM region as runtime memory
  Status = gDS->GetMemorySpaceDescriptor(
      ROUND_TO_PAGE(MPM2_MPM_PS_HOLD), &psHoldMemoryDescriptor);

  if (EFI_ERROR(Status))
    goto exit;

  if (!(psHoldMemoryDescriptor.Attributes & EFI_MEMORY_RUNTIME)) {
    Status = gDS->SetMemorySpaceAttributes(
        ROUND_TO_PAGE(MPM2_MPM_PS_HOLD), EFI_PAGE_SIZE,
        psHoldMemoryDescriptor.Attributes | EFI_MEMORY_RUNTIME);

    if (EFI_ERROR(Status))
      goto exit;
  }

  Status = gDS->GetMemorySpaceDescriptor(
      ROUND_TO_PAGE(MSM_SHARED_IMEM_BASE), &imemRegionMemoryDescriptor);

  if (EFI_ERROR(Status))
    goto exit;

  if (!(imemRegionMemoryDescriptor.Attributes & EFI_MEMORY_RUNTIME)) {
    Status = gDS->SetMemorySpaceAttributes(
        ROUND_TO_PAGE(MSM_SHARED_IMEM_BASE), EFI_PAGE_SIZE,
        imemRegionMemoryDescriptor.Attributes | EFI_MEMORY_RUNTIME);

    if (EFI_ERROR(Status))
      goto exit;
  }

#if SILICON_PLATFORM == 8992
  Status = gDS->GetMemorySpaceDescriptor(
      ROUND_TO_PAGE(MSM_SHARED_IMEM_BASE2), &imemRegionMemoryDescriptor);

  if (EFI_ERROR(Status))
    goto exit;

  if (!(imemRegionMemoryDescriptor.Attributes & EFI_MEMORY_RUNTIME)) {
    Status = gDS->SetMemorySpaceAttributes(
        ROUND_TO_PAGE(MSM_SHARED_IMEM_BASE2), EFI_PAGE_SIZE,
        imemRegionMemoryDescriptor.Attributes | EFI_MEMORY_RUNTIME);

    if (EFI_ERROR(Status))
      goto exit;
  }
#endif

  Status = gDS->GetMemorySpaceDescriptor(
      ROUND_TO_PAGE(SPMI_BASE), &spmiMemoryDescriptor);

  if (EFI_ERROR(Status))
    goto exit;

  if (!(spmiMemoryDescriptor.Attributes & EFI_MEMORY_RUNTIME)) {
    Status = gDS->SetMemorySpaceAttributes(
        ROUND_TO_PAGE(SPMI_BASE),
        // HRD told us it is 64KB
        0x10000, spmiMemoryDescriptor.Attributes | EFI_MEMORY_RUNTIME);

    if (EFI_ERROR(Status))
      goto exit;
  }

  // If everything is good, you can securely call ResetSystem
  // in RT phase.

exit:
  return Status;
}