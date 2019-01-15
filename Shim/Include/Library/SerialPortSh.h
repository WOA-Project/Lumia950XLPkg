#ifndef __SERIAL_PORT_SH_H__
#define __SERIAL_PORT_SH_H__

#include <Uefi.h>

/* Name for the SerialPort Shared Library Interface */
#define SIO_PORT_LIB_NAME "SerialPort Lib"

/* Corresponds to version 1.1
 * MSB 16 bits Major version
 * LSB 16 bits Minor version
 * See note about the versioning and the way Loader works in LOAD_LIB comments
 * In short, if major version is same the lib should be "strictly compatible"
 * with all the previous versions */
#define SIO_PORT_LIB_VERSION  0x00010001

 /* Port Control Option Arguments and Param values (Refer to SIO_CONTROL)
  *   Arg :
  *     SIO_CONTROL_BUFFERING
  *       Control Buffering of the output log data,
  *          Param : FALSE - Disables buffering
  *          Param : TRUE  - Enables buffering
  *
  *     SIO_CONTROL_PORTOUT
  *       Control Data output through Serial Port
  *          Param : FALSE - Disables Data output via serial port
  *          Param : TRUE  - Enables Data output via serial port
  *
  *     SIO_CONTROL_SYNCHRONOUS_IO
  *       Configure if Synchronous IO will be performed.
  *          Param : TRUE  - Enables Blocked IO (doesn't return until the data is sent).
  *          Param : FALSE - Disables Blocked IO
  * */
#define SIO_CONTROL_BUFFERING        0x01
#define SIO_CONTROL_PORTOUT          0x02
#define SIO_CONTROL_SYNCHRONOUS_IO   0x03

  /* Typedefs for SerialPort Shared Library Interface */
typedef UINTN(*SIO_READ) (OUT  UINT8  *Buffer, IN UINTN NumberOfBytes);
typedef UINTN(*SIO_WRITE) (IN  UINT8  *Buffer, IN UINTN NumberOfBytes);
typedef BOOLEAN(*SIO_POLL) (VOID);
typedef UINTN(*SIO_DRAIN) (VOID);
typedef UINTN(*SIO_FLUSH) (VOID);
typedef UINTN(*SIO_CONTROL) (IN UINTN Arg, IN UINTN Param);

/* SerialPort Shared Library Interface */
typedef struct {
	UINT32       LibVersion;
	SIO_READ     Read;
	SIO_WRITE    Write;
	SIO_POLL     Poll;
	SIO_DRAIN    Drain;
	SIO_FLUSH    Flush;
	SIO_CONTROL  Control;
}SioPortLibType;

/* Loader function that helps getting the Serial Port Lib interface */
INTN LoadSioLib(
	IN  SioPortLibType* SioLibIntfPtr OPTIONAL,
	IN  UINT32          LibVersion    OPTIONAL
);

/******************************************************************
 *
 *  Below are specific to the shared lib implementation
 *
 ******************************************************************/

UINTN SerialPortDrain(VOID);

UINTN SerialPortFlush(VOID);

UINTN SerialPortControl(
	IN UINTN Arg,
	IN UINTN Param
);

VOID EnableSynchronousSerialPortIO(VOID);

VOID DisableSerialOut(VOID);

#endif