#include "AcpiTableInstaller.h"

STATIC EFI_GUID gAcpiTablesPsci = {
    0x7e374e25,
    0x8e01,
    0x4fee,
    {
        0x87,
        0xf2,
        0x39,
        0xc,
        0x23,
        0xc6,
        0x6,
        0xb1,
    },
};

STATIC EFI_GUID gAcpiTablesMpPark = {
    0x7e374e25,
    0x8e01,
    0x4fee,
    {
        0x87,
        0xf2,
        0x39,
        0xc,
        0x23,
        0xc6,
        0x6,
        0xb2,
    },
};

STATIC EFI_GUID gAcpiTablesMpParkQuadCore = {
    0x7e374e25,
    0x8e01,
    0x4fee,
    {
        0x87,
        0xf2,
        0x39,
        0xc,
        0x23,
        0xc6,
        0x6,
        0xb3,
    },
};

STATIC EFI_GUID gAcpiTablesEmmcBoot = {
    0x7e374e25,
    0x8e01,
    0x4fee,
    {
        0x87,
        0xf2,
        0x39,
        0xc,
        0x23,
        0xc6,
        0x6,
        0xb4,
    },
};

STATIC EFI_GUID gAcpiTablesSdBoot = {
    0x7e374e25,
    0x8e01,
    0x4fee,
    {
        0x87,
        0xf2,
        0x39,
        0xc,
        0x23,
        0xc6,
        0x6,
        0xb5,
    },
};

/**
  Locate the first instance of a protocol.  If the protocol requested is an
  FV protocol, then it will return the first FV that contains the ACPI table
  storage file.
  @param  Guid          The guid of the ACPI storage file
  @param  Instance      Return pointer to the first instance of the protocol
  @return EFI_SUCCESS           The function completed successfully.
  @return EFI_NOT_FOUND         The protocol could not be located.
  @return EFI_OUT_OF_RESOURCES  There are not enough resources to find the
protocol.
**/
EFI_STATUS
LocateFvInstanceWithTables(
    IN EFI_GUID Guid, OUT EFI_FIRMWARE_VOLUME2_PROTOCOL **Instance)
{
  EFI_STATUS                     Status;
  EFI_HANDLE *                   HandleBuffer;
  UINTN                          NumberOfHandles;
  EFI_FV_FILETYPE                FileType;
  UINT32                         FvStatus;
  EFI_FV_FILE_ATTRIBUTES         Attributes;
  UINTN                          Size;
  UINTN                          Index;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *FvInstance;

  FvStatus = 0;

  //
  // Locate protocol.
  //
  Status = gBS->LocateHandleBuffer(
      ByProtocol, &gEfiFirmwareVolume2ProtocolGuid, NULL, &NumberOfHandles,
      &HandleBuffer);
  if (EFI_ERROR(Status)) {
    //
    // Defined errors at this time are not found and out of resources.
    //
    return Status;
  }

  //
  // Looking for FV with ACPI storage file
  //

  for (Index = 0; Index < NumberOfHandles; Index++) {
    //
    // Get the protocol on this handle
    // This should not fail because of LocateHandleBuffer
    //
    Status = gBS->HandleProtocol(
        HandleBuffer[Index], &gEfiFirmwareVolume2ProtocolGuid,
        (VOID **)&FvInstance);
    ASSERT_EFI_ERROR(Status);

    //
    // See if it has the ACPI storage file
    //
    Status = FvInstance->ReadFile(
        FvInstance, &Guid, NULL, &Size, &FileType, &Attributes, &FvStatus);

    //
    // If we found it, then we are done
    //
    if (Status == EFI_SUCCESS) {
      *Instance = FvInstance;
      break;
    }
  }

  //
  // Our exit status is determined by the success of the previous operations
  // If the protocol was found, Instance already points to it.
  //

  //
  // Free any allocated buffers
  //
  gBS->FreePool(HandleBuffer);

  return Status;
}

/**
  This function calculates and updates an UINT8 checksum.
  @param  Buffer          Pointer to buffer to checksum
  @param  Size            Number of bytes to checksum
**/
VOID AcpiPlatformChecksum(IN UINT8 *Buffer, IN UINTN Size)
{
  UINTN ChecksumOffset;

  ChecksumOffset = OFFSET_OF(EFI_ACPI_DESCRIPTION_HEADER, Checksum);

  //
  // Set checksum to 0 first
  //
  Buffer[ChecksumOffset] = 0;

  //
  // Update checksum value
  //
  Buffer[ChecksumOffset] = CalculateCheckSum8(Buffer, Size);
}

EFI_STATUS
EFIAPI
LoadAcpiTablesFromGuid(IN EFI_GUID Guid)
{
  EFI_STATUS                     Status;
  EFI_ACPI_TABLE_PROTOCOL *      AcpiTable;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *FwVol;
  INTN                           Instance;
  EFI_ACPI_COMMON_HEADER *       CurrentTable;
  UINTN                          TableHandle;
  UINT32                         FvStatus;
  UINTN                          TableSize;
  UINTN                          Size;

  Instance     = 0;
  CurrentTable = NULL;
  TableHandle  = 0;

  //
  // Find the AcpiTable protocol
  //
  Status = gBS->LocateProtocol(
      &gEfiAcpiTableProtocolGuid, NULL, (VOID **)&AcpiTable);
  if (EFI_ERROR(Status)) {
    return EFI_ABORTED;
  }

  //
  // Locate the firmware volume protocol
  //
  Status = LocateFvInstanceWithTables(Guid, &FwVol);
  if (EFI_ERROR(Status)) {
    return EFI_ABORTED;
  }
  //
  // Read tables from the storage file.
  //
  while (Status == EFI_SUCCESS) {

    Status = FwVol->ReadSection(
        FwVol, &Guid, EFI_SECTION_RAW, Instance, (VOID **)&CurrentTable, &Size,
        &FvStatus);
    if (!EFI_ERROR(Status)) {
      //
      // Add the table
      //
      TableHandle = 0;

      TableSize = ((EFI_ACPI_DESCRIPTION_HEADER *)CurrentTable)->Length;
      ASSERT(Size >= TableSize);

      //
      // Checksum ACPI table
      //
      AcpiPlatformChecksum((UINT8 *)CurrentTable, TableSize);

      //
      // Install ACPI table
      //
      Status = AcpiTable->InstallAcpiTable(
          AcpiTable, CurrentTable, TableSize, &TableHandle);

      //
      // Free memory allocated by ReadSection
      //
      gBS->FreePool(CurrentTable);

      if (EFI_ERROR(Status)) {
        return EFI_ABORTED;
      }

      //
      // Increment the instance
      //
      Instance++;
      CurrentTable = NULL;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InstallAcpiTables()
{
  PPRELOADER_ENVIRONMENT Env    = (VOID *)PRELOADER_ENV_ADDR;
  EFI_STATUS             Status = EFI_SUCCESS;

  switch (Env->BootMode) {
  case BOOT_MODE_MPPARK:
  case BOOT_MODE_MPPARK_EL2: {
    if (Env->UseQuadCoreConfiguration) {
      Status = LoadAcpiTablesFromGuid(gAcpiTablesMpParkQuadCore);
      if (EFI_ERROR(Status)) {
        goto exit;
      }
    }
    else {
      Status = LoadAcpiTablesFromGuid(gAcpiTablesMpPark);
      if (EFI_ERROR(Status)) {
        goto exit;
      }
    }
    break;
  }
  case BOOT_MODE_PSCI:
  default: {
    Status = LoadAcpiTablesFromGuid(gAcpiTablesPsci);
    if (EFI_ERROR(Status)) {
      goto exit;
    }
    break;
  }
  }

  if (Env->EnablePlatformSdCardBoot) {
    Status = LoadAcpiTablesFromGuid(gAcpiTablesSdBoot);
    if (EFI_ERROR(Status)) {
      goto exit;
    }
  }
  else {
    Status = LoadAcpiTablesFromGuid(gAcpiTablesEmmcBoot);
    if (EFI_ERROR(Status)) {
      goto exit;
    }
  }

exit:
  return Status;
}