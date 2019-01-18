/* SimpleFbDxe: Simple FrameBuffer */
#include <PiDxe.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Library/BaseLib.h>

/// Defines
/*
 * Convert enum video_log2_bpp to bytes and bits. Note we omit the outer
 * brackets to allow multiplication by fractional pixels.
 */
#define VNBYTES(bpix)	(1 << (bpix)) / 8
#define VNBITS(bpix)	(1 << (bpix))

#define POS_TO_FB(posX, posY) ((UINT8 *)                                \
                               ((UINTN)This->Mode->FrameBufferBase +    \
                                (posY) * This->Mode->Info->PixelsPerScanLine * \
                                FB_BYTES_PER_PIXEL +                   \
                                (posX) * FB_BYTES_PER_PIXEL))

#define FB_BITS_PER_PIXEL                   (32)
#define FB_BYTES_PER_PIXEL                  (FB_BITS_PER_PIXEL / 8)
#define DISPLAYDXE_PHYSICALADDRESS32(_x_)   (UINTN)((_x_) & 0xFFFFFFFF)

#define DISPLAYDXE_RED_MASK                0xFF0000
#define DISPLAYDXE_GREEN_MASK              0x00FF00
#define DISPLAYDXE_BLUE_MASK               0x0000FF
#define DISPLAYDXE_ALPHA_MASK              0x000000

/*
 * Bits per pixel selector. Each value n is such that the bits-per-pixel is
 * 2 ^ n
 */
enum video_log2_bpp {
	VIDEO_BPP1	= 0,
	VIDEO_BPP2,
	VIDEO_BPP4,
	VIDEO_BPP8,
	VIDEO_BPP16,
	VIDEO_BPP32,
};

typedef struct {
  VENDOR_DEVICE_PATH DisplayDevicePath;
  EFI_DEVICE_PATH EndDevicePath;
} DISPLAY_DEVICE_PATH;

DISPLAY_DEVICE_PATH mDisplayDevicePath =
{
    {
      {
        HARDWARE_DEVICE_PATH,
        HW_VENDOR_DP,
        {
          (UINT8)(sizeof(VENDOR_DEVICE_PATH)),
          (UINT8)((sizeof(VENDOR_DEVICE_PATH)) >> 8),
        }
      },
      EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID
    },
    {
      END_DEVICE_PATH_TYPE,
      END_ENTIRE_DEVICE_PATH_SUBTYPE,
      {
        sizeof(EFI_DEVICE_PATH_PROTOCOL),
        0
      }
    }
};

/// Declares

STATIC
EFI_STATUS
EFIAPI
DisplayQueryMode
(
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
    IN  UINT32                                ModeNumber,
    OUT UINTN                                 *SizeOfInfo,
    OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
);

STATIC
EFI_STATUS
EFIAPI
DisplaySetMode
(
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN  UINT32                       ModeNumber
);

STATIC
EFI_STATUS
EFIAPI
DisplayBlt
(
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL            *This,
    IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL           *BltBuffer,   OPTIONAL
    IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION       BltOperation,
    IN  UINTN                                   SourceX,
    IN  UINTN                                   SourceY,
    IN  UINTN                                   DestinationX,
    IN  UINTN                                   DestinationY,
    IN  UINTN                                   Width,
    IN  UINTN                                   Height,
    IN  UINTN                                   Delta         OPTIONAL
);

/* Display Blit function */
static void DisplayDxeBltInternal 
(
  UINT8   *pSrc,
  UINT8   *pDst,
  UINTN    uSrcX,
  UINTN    uSrcY,
  UINTN    uSrcWidth,
  UINTN    uSrcHeight,
  UINTN    uSrcStride,
  UINTN    uDstX,
  UINTN    uDstY,
  UINTN    uDstStride,
  UINTN    uBytesPerPixel
);

STATIC EFI_GRAPHICS_OUTPUT_PROTOCOL mDisplay = {
  DisplayQueryMode,
  DisplaySetMode,
  DisplayBlt,
  NULL
};

static void DisplayDxeBltInternal 
(
  UINT8   *pSrc,
  UINT8   *pDst,
  UINTN    uSrcX,
  UINTN    uSrcY,
  UINTN    uSrcWidth,
  UINTN    uSrcHeight,
  UINTN    uSrcStride,
  UINTN    uDstX,
  UINTN    uDstY,
  UINTN    uDstStride,
  UINTN    uBytesPerPixel
  )
{
  UINT32 uI = 0;
  UINT32 uSrcWidthBytes = uSrcWidth * uBytesPerPixel;  
  
  /* move src pointer to start of src rectangle */
  pSrc += (uSrcY * uSrcStride) + (uSrcX * uBytesPerPixel);

  /* move dest pointer to start of dest rectangle */
  pDst += (uDstY * uDstStride) + (uDstX * uBytesPerPixel); 

  /* Blit Operation 
   *
   *  We use MDP_OSAL_MEMCPY which invokes Neon memcpy (kr_memcpy.asm) 
   *  This memcpy supports overlapped src and dst buffers but copying may not be optimal in the overlap case 
   */  
  for (uI = 0; uI < uSrcHeight; ++uI)
  {
    gBS->CopyMem((VOID*) pDst, (VOID*) pSrc, uSrcWidthBytes);

    pDst += uDstStride;
    pSrc += uSrcStride;
  }
}

STATIC
EFI_STATUS
EFIAPI
DisplayQueryMode
(
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
    IN  UINT32                                ModeNumber,
    OUT UINTN                                 *SizeOfInfo,
    OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
)
{
    EFI_STATUS Status;
    Status = gBS->AllocatePool(
        EfiBootServicesData,
        sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
        (VOID **) Info);

    ASSERT_EFI_ERROR(Status);

    *SizeOfInfo = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    (*Info)->Version = This->Mode->Info->Version;
    (*Info)->HorizontalResolution = This->Mode->Info->HorizontalResolution;
    (*Info)->VerticalResolution = This->Mode->Info->VerticalResolution;
    (*Info)->PixelFormat = This->Mode->Info->PixelFormat;
    (*Info)->PixelsPerScanLine = This->Mode->Info->PixelsPerScanLine;

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
DisplaySetMode
(
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN  UINT32                       ModeNumber
)
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
DisplayBlt
(
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL      *This,
    IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL     *BltBuffer,   OPTIONAL
    IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
    IN  UINTN                             SourceX,
    IN  UINTN                             SourceY,
    IN  UINTN                             DestinationX,
    IN  UINTN                             DestinationY,
    IN  UINTN                             Width,
    IN  UINTN                             Height,
    IN  UINTN                             Delta         OPTIONAL
)
{
    EFI_STATUS Status = EFI_SUCCESS;

    if (NULL == This)
    {
        Status = EFI_INVALID_PARAMETER;
    }
    else
    {
        switch (BltOperation)
        {
        case EfiBltBufferToVideo:
        {
            UINT8 *pSrcBuffer = (UINT8*) BltBuffer;
            UINT8 *pDstBuffer = (UINT8*) DISPLAYDXE_PHYSICALADDRESS32(mDisplay.Mode->FrameBufferBase);
            UINTN SrcStride, DstStride, CopyWidth, CopyHeight;

            if (((DestinationX + Width) > mDisplay.Mode->Info->HorizontalResolution) ||
                ((DestinationY + Height) > mDisplay.Mode->Info->VerticalResolution))
            {
                return EFI_INVALID_PARAMETER;
            }

            CopyWidth  = Width;
            CopyHeight = Height;

            /* Video buffer stride in bytes, consider padding as well */
            DstStride = mDisplay.Mode->Info->PixelsPerScanLine * FB_BYTES_PER_PIXEL;

            /* Src buffer stride in bytes. Delta is valid when X or Y is not 0 */
            SrcStride = Width * FB_BYTES_PER_PIXEL;
            if (Delta != 0)
            {  
                SrcStride = Delta;
            }

            DisplayDxeBltInternal(pSrcBuffer,
                pDstBuffer,
                SourceX, 
                SourceY, 
                CopyWidth, 
                CopyHeight,
                SrcStride,
                DestinationX,
                DestinationY,
                DstStride,
                FB_BYTES_PER_PIXEL);
          
            Status = EFI_SUCCESS;
        }
        break;

        case EfiBltVideoToBltBuffer:
        {  
            UINT8 *pSrcBuffer = (UINT8*) BltBuffer;
            UINT8 *pDstBuffer = (UINT8*) DISPLAYDXE_PHYSICALADDRESS32(mDisplay.Mode->FrameBufferBase);
            UINTN SrcStride, DstStride, CopyWidth, CopyHeight;

            if (((SourceX + Width) > mDisplay.Mode->Info->HorizontalResolution) ||
                ((SourceY + Height) > mDisplay.Mode->Info->VerticalResolution))
            {
                return EFI_INVALID_PARAMETER;
            }

            CopyWidth  = Width;
            CopyHeight = Height;

            /* Video buffer stride in bytes, consider padding as well */
            SrcStride = mDisplay.Mode->Info->PixelsPerScanLine * FB_BYTES_PER_PIXEL;

            /* Buffer stride in bytes. Delta is valid when X or Y is not 0 */
            DstStride = Width * FB_BYTES_PER_PIXEL;
            if (Delta != 0)
            {
                DstStride = Delta;
            }

            DisplayDxeBltInternal(pSrcBuffer,
                pDstBuffer,
                SourceX, 
                SourceY, 
                CopyWidth, 
                CopyHeight,
                SrcStride,
                DestinationX,
                DestinationY,
                DstStride,
                FB_BYTES_PER_PIXEL);

            Status = EFI_SUCCESS;
        }
        break; 

        case EfiBltVideoFill:
        {
            UINT32 SrcPixel = *(UINT32*) BltBuffer;
            UINT32 *pDstBuffer = (UINT32*) DISPLAYDXE_PHYSICALADDRESS32(mDisplay.Mode->FrameBufferBase);
            UINTN  DstStride, CopyWidth, CopyHeight;
            UINT32 x,y;

            if (((DestinationX + Width) > mDisplay.Mode->Info->HorizontalResolution) ||
                ((DestinationY + Height) > mDisplay.Mode->Info->VerticalResolution))
            {
                return EFI_INVALID_PARAMETER;
            }

            CopyWidth  = Width;
            CopyHeight = Height;

            /* Video buffer stride in bytes, consider padding as well */
            DstStride = mDisplay.Mode->Info->PixelsPerScanLine * FB_BYTES_PER_PIXEL;

            /* Adjust Destination location */
            pDstBuffer = (UINT32*)(((UINT8*)pDstBuffer) + (DestinationY * DstStride) + DestinationX * FB_BYTES_PER_PIXEL); 

            /* Do the actual blitting */
            for (y = 0; y < CopyHeight; y++)
            {
                for (x = 0; x < CopyWidth; x++)
                {
                    pDstBuffer[x] = SrcPixel;
                }

                /* Increment by stride number of bytes */
                pDstBuffer = (UINT32*)((UINT8*)pDstBuffer + DstStride);
            }
            Status = EFI_SUCCESS;
        }
        break;

        case EfiBltVideoToVideo:
        {
            UINT8 *pSrcBuffer = (UINT8*) DISPLAYDXE_PHYSICALADDRESS32(mDisplay.Mode->FrameBufferBase);
            UINT8 *pDstBuffer = (UINT8*) DISPLAYDXE_PHYSICALADDRESS32(mDisplay.Mode->FrameBufferBase);
            UINTN  Stride, CopyWidth, CopyHeight;

            if (((SourceX + Width) > mDisplay.Mode->Info->HorizontalResolution))
                Width = mDisplay.Mode->Info->HorizontalResolution - SourceX;

            if (((SourceY + Height) > mDisplay.Mode->Info->VerticalResolution))
                Height = mDisplay.Mode->Info->VerticalResolution - SourceY;

            if (((DestinationX + Width) > mDisplay.Mode->Info->HorizontalResolution))
                Width = mDisplay.Mode->Info->HorizontalResolution - DestinationX;

            if (((DestinationY + Height) > mDisplay.Mode->Info->VerticalResolution))
                Height = mDisplay.Mode->Info->VerticalResolution - DestinationY;

            CopyWidth  = Width;
            CopyHeight = Height;

            /* Video buffer stride in bytes, consider padding as well */
            Stride = mDisplay.Mode->Info->PixelsPerScanLine * FB_BYTES_PER_PIXEL;

            DisplayDxeBltInternal(pSrcBuffer,
                pDstBuffer,
                SourceX, 
                SourceY, 
                CopyWidth, 
                CopyHeight,
                Stride,
                DestinationX,
                DestinationY,
                Stride,
                FB_BYTES_PER_PIXEL);

            Status = EFI_SUCCESS;

        }
        break;

        default:
            DEBUG ((EFI_D_ERROR, "SimpleFbDxe: BltOperation not supported\n"));
            Status = RETURN_INVALID_PARAMETER;
            break;
        }    
    }

    return Status;    
}

EFI_STATUS
EFIAPI
SimpleFbDxeInitialize
(
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
{

    EFI_STATUS          Status                  = EFI_SUCCESS;
    EFI_HANDLE          hUEFIDisplayHandle      = NULL;

    /* Retrieve simple frame buffer from pre-SEC bootloader */
    DEBUG((EFI_D_ERROR, "SimpleFbDxe: Retrieve MIPI FrameBuffer parameters from PCD\n"));
    UINT32              MipiFrameBufferAddr     = FixedPcdGet32(PcdMipiFrameBufferAddress);
    UINT32              MipiFrameBufferWidth    = FixedPcdGet32(PcdMipiFrameBufferWidth);
    UINT32              MipiFrameBufferHeight   = FixedPcdGet32(PcdMipiFrameBufferHeight);

    /* Sanity check */
    if (MipiFrameBufferAddr == 0 || MipiFrameBufferWidth == 0 || MipiFrameBufferHeight == 0)
    {
        DEBUG((EFI_D_ERROR, "SimpleFbDxe: Invalid FrameBuffer parameters\n"));
        return EFI_DEVICE_ERROR;
    }

    /* Prepare struct */
    if (mDisplay.Mode == NULL)
    {
        Status = gBS->AllocatePool(
            EfiBootServicesData,
            sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE),
            (VOID **) &mDisplay.Mode
        );

        ASSERT_EFI_ERROR(Status);
        if (EFI_ERROR(Status)) return Status;

        ZeroMem(mDisplay.Mode, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE));
    }
    
    if (mDisplay.Mode->Info == NULL)
    {
        Status = gBS->AllocatePool(
            EfiBootServicesData,
            sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
            (VOID **) &mDisplay.Mode->Info
        );

        ASSERT_EFI_ERROR(Status);
        if (EFI_ERROR(Status)) return Status;

        ZeroMem(mDisplay.Mode->Info, sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
    }

    /* Set information */
    mDisplay.Mode->MaxMode = 1;
    mDisplay.Mode->Mode = 0;
    mDisplay.Mode->Info->Version = 0;

    mDisplay.Mode->Info->HorizontalResolution = MipiFrameBufferWidth;
    mDisplay.Mode->Info->VerticalResolution = MipiFrameBufferHeight;

    /* SimpleFB runs on a8r8g8b8 (VIDEO_BPP32) for DB410c */
    UINT32 LineLength = MipiFrameBufferWidth * VNBYTES(VIDEO_BPP32);
    UINT32 FrameBufferSize = LineLength * MipiFrameBufferHeight;
    EFI_PHYSICAL_ADDRESS FrameBufferAddress = MipiFrameBufferAddr;

    mDisplay.Mode->Info->PixelsPerScanLine = MipiFrameBufferWidth;
    mDisplay.Mode->Info->PixelFormat = PixelBlueGreenRedReserved8BitPerColor;
    mDisplay.Mode->SizeOfInfo = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    mDisplay.Mode->FrameBufferBase = FrameBufferAddress;
    mDisplay.Mode->FrameBufferSize = FrameBufferSize;

    /* Register handle */
    Status = gBS->InstallMultipleProtocolInterfaces(
        &hUEFIDisplayHandle,
        &gEfiDevicePathProtocolGuid,
        &mDisplayDevicePath,
        &gEfiGraphicsOutputProtocolGuid,
        &mDisplay,
        NULL);

    ASSERT_EFI_ERROR (Status);

    return Status;

}