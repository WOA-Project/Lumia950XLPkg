#include "Ramdisk.h"

#define RAM_BIO_INSTANCE_SIGNATURE  SIGNATURE_32('e', 'm', 'm', 'c')
#define RAM_BIO_BLKSIZE 512

BOOLEAN mDeviceReady = FALSE;
STATIC EFI_EVENT mRtcVirtualAddrChangeEvent;

// Static media info
STATIC EFI_BLOCK_IO_MEDIA mMediaInfo = 
{
	RAM_BIO_INSTANCE_SIGNATURE,								// MediaId
	FALSE,													// RemovableMedia
	TRUE,													// MediaPresent
	FALSE,													// LogicalPartition
	FALSE,													// ReadOnly
	FALSE,													// WriteCaching
	RAM_BIO_BLKSIZE,										// BlockSize
	4,														// IoAlign
	0,														// Pad
	(FixedPcdGetSize(PcdRamdiskSize) / RAM_BIO_BLKSIZE)		// LastBlock
};

/**
Fixup internal data so that EFI can be call in virtual mode.
Call the passed in Child Notify event and convert any pointers in
lib to virtual mode.

@param[in]    Event   The Event that is being processed
@param[in]    Context Event Context
**/
VOID
EFIAPI
LibRtcVirtualNotifyEvent(
	IN EFI_EVENT        Event,
	IN VOID             *Context
)
{
	//
	// Only needed if you are going to support the OS calling RTC functions in virtual mode.
	// You will need to call EfiConvertPointer (). To convert any stored physical addresses
	// to virtual address. After the OS transitions to calling in virtual mode, all future
	// runtime calls will be made in virtual mode.
	//
	mDeviceReady = FALSE;
	return;
}

EFI_STATUS
EFIAPI
RamdiskReset
(
	IN EFI_BLOCK_IO_PROTOCOL          *This,
	IN BOOLEAN                        ExtendedVerification
)
{
	if (!mDeviceReady) return EFI_NOT_READY;

	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RamdiskReadBlocks
(
	IN EFI_BLOCK_IO_PROTOCOL          *This,
	IN UINT32                         MediaId,
	IN EFI_LBA                        Lba,
	IN UINTN                          BufferSize,
	OUT VOID                          *Buffer
)
{
	UINTN BlockSize = mMediaInfo.BlockSize;
	UINT64 LowerBoundary = FixedPcdGet64(PcdRamdiskAddress);
	UINT64 UpperBoundary = FixedPcdGet64(PcdRamdiskAddress) + FixedPcdGet64(PcdRamdiskSize);
	UINT64 ReadBegin = 0;
	UINT64 ReadEnd = 0;
	VOID* RamdiskAddress;

	// This Media
	if (MediaId != mMediaInfo.MediaId)
	{
		return EFI_MEDIA_CHANGED;
	}

	// Check
	if (!mDeviceReady) return EFI_NOT_READY;
	
	// Overlap check
	ReadBegin = LowerBoundary + Lba * BlockSize;
	ReadEnd = ReadBegin + BufferSize;

	if (ReadBegin > UpperBoundary || ReadEnd > UpperBoundary)
	{
		DEBUG((EFI_D_ERROR, "Out of bound read attempt, LBA = %d, Buffer = %d", Lba, BufferSize));
		return EFI_INVALID_PARAMETER;
	}

	// Alignment
	if (BufferSize % BlockSize != 0) {
		DEBUG((EFI_D_ERROR, "Non-aligned read attempt"));
		return EFI_BAD_BUFFER_SIZE;
	}

	// Buffer check
	if (Buffer == NULL) 
	{
		DEBUG((EFI_D_ERROR, "Null buffer read attempt"));
		return EFI_INVALID_PARAMETER;
	}

	if (BufferSize == 0) 
	{
		return EFI_SUCCESS;
	}

	// Copy memory to buffer.
	RamdiskAddress = (VOID *) (FixedPcdGet64(PcdRamdiskAddress) + Lba * BlockSize);
	CopyMem(Buffer, RamdiskAddress, BufferSize);

	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RamdiskWriteBlocks
(
	IN EFI_BLOCK_IO_PROTOCOL          *This,
	IN UINT32                         MediaId,
	IN EFI_LBA                        Lba,
	IN UINTN                          BufferSize,
	IN VOID                           *Buffer
)
{
	// Check
	if (!mDeviceReady) return EFI_NOT_READY;

	return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
RamdiskFlushBlocks
(
	IN EFI_BLOCK_IO_PROTOCOL  *This
)
{
	// Check
	if (!mDeviceReady) return EFI_NOT_READY;

	return EFI_SUCCESS;
}


STATIC EFI_BLOCK_IO_PROTOCOL mBlkIoProtocol = 
{
	EFI_BLOCK_IO_INTERFACE_REVISION,		// Revision
	&mMediaInfo,							// *Media
	RamdiskReset,							// Reset
	RamdiskReadBlocks,						// ReadBlocks
	RamdiskWriteBlocks,						// WriteBlocks
	RamdiskFlushBlocks						// FlushBlocks
};

EFI_STATUS
EFIAPI
RamdiskInitialize
(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS Status = EFI_SUCCESS;
	EFI_HANDLE hRamdisk = NULL;
	
	// Install protocol
	Status = gBS->InstallMultipleProtocolInterfaces(
		&hRamdisk,
		&gEfiBlockIoProtocolGuid,
		&mBlkIoProtocol,
		&gEfiDevicePathProtocolGuid,
		&mRamdiskDevicePath,
		NULL
	);

	if (EFI_ERROR(Status))
	{
		DEBUG((EFI_D_ERROR, "Failed to install Ramdisk device\n"));
		goto exit;
	}

	// Check address change
	Status = gBS->CreateEventEx(
		EVT_NOTIFY_SIGNAL,
		TPL_NOTIFY,
		LibRtcVirtualNotifyEvent,
		NULL,
		&gEfiEventVirtualAddressChangeGuid,
		&mRtcVirtualAddrChangeEvent
	);

	if (EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "Failed to register event.\n"));
		return Status;
	}

	mDeviceReady = TRUE;
	DEBUG((EFI_D_ERROR, "Ramdisk device installed!!!\n"));

exit:
	return Status;
}
