#include <PiPei.h>

#include <Library/LKEnvLib.h>
#include <Library/SerialPortSh.h>

/* Version 1.1 */
#define CURRENT_SIO_LIB_VERSION  0x00010001

extern
UINTN
EFIAPI
SerialPortWrite
(
	IN UINT8     *Buffer,
	IN UINTN     NumberOfBytes
);

UINTN 
EFIAPI
ShimSerialPortRead(
	OUT UINT8 *user_buffer, 
	IN UINTN bytes_requested
)
{
	return 0;
}

UINTN
ShimSerialPortWrite(
	UINT8* Buffer, 
	UINTN Bytes
)
{
	return SerialPortWrite(Buffer, Bytes);
}

UINTN
ShimSerialPortDrain(VOID)
{
	return 0;
}

UINTN
ShimSerialPortFlush(VOID)
{
	return 0;
}

BOOLEAN 
EFIAPI
ShimSerialPortPoll(void)
{
	return FALSE;
}

UINTN
ShimSerialPortControl(
	IN UINTN Arg, 
	IN UINTN Param
)
{
	return 0;
}

SioPortLibType SioLib =
{
	.LibVersion = CURRENT_SIO_LIB_VERSION,
	.Read = ShimSerialPortRead,
	.Write = ShimSerialPortWrite,
	.Drain = ShimSerialPortDrain,
	.Flush = ShimSerialPortFlush,
	.Poll = ShimSerialPortPoll,
	.Control = ShimSerialPortControl,
};
