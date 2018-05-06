#include <PiDxe.h>

#include <Library/LKEnvLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/QcomI2cQup.h>
#include <Protocol/QcomGpioTlmm.h>
#include <Library/PcdLib.h>

#include "SynapticsRmi4.h"

QCOM_GPIO_TLMM_PROTOCOL *GpioTlmmProtocol;
QCOM_I2C_QUP_PROTOCOL *I2cQupProtocol;

BOOLEAN m_DeviceInitialized = FALSE;
struct qup_i2c_dev *m_Controller;

EFI_STATUS
EFIAPI
SynaPowerUpController(
	VOID
)
{
	UINT32 ResetLine;
	EFI_STATUS Status;

	if (GpioTlmmProtocol == NULL)
	{
		Status = EFI_NOT_READY;
		goto exit;
	}

	// Pin Sanity check
	ResetLine = FixedPcdGet32(SynapticsCtlrResetPin);
	if (ResetLine <= 0)
	{
		DEBUG((EFI_D_ERROR, "Invalid GPIO configuration \n"));
		Status = EFI_INVALID_PARAMETER;
		goto exit;
	}

	// Power Seq
	GpioTlmmProtocol->SetPull(ResetLine, GPIO_PULL_NONE);
	GpioTlmmProtocol->SetDriveStrength(ResetLine, 2);
	GpioTlmmProtocol->Set(ResetLine, GPIO_ENABLE);

	// Configure MSM GPIO RESET line to Low
	GpioTlmmProtocol->DirectionOutput(ResetLine, GPIO_LOW);
	gBS->Stall(TOUCH_POWER_RAIL_STABLE_TIME);

	// configure MSM GPIO RESET line to High
	GpioTlmmProtocol->DirectionOutput(ResetLine, GPIO_HIGH);
	gBS->Stall(TOUCH_DELAY_TO_COMMUNICATE);

	DEBUG((EFI_D_INFO, "Touch controller powered on \n"));
	Status = EFI_SUCCESS;

exit:
	return Status;
}

EFI_STATUS
EFIAPI
SynaI2cRead(
	IN UINT8				Address,
	IN UINT8*				Data,
	IN UINT16				ReadBytes
)
{
	EFI_STATUS Status = EFI_SUCCESS;
	int Transferred = 0;

	if (m_Controller == NULL || I2cQupProtocol == NULL)
	{
		Status = EFI_NOT_READY;
		goto exit;
	}

	if (Data == NULL)
	{
		Status = EFI_INVALID_PARAMETER;
		goto exit;
	}

	struct i2c_msg ControllerProbeMsg[] =
	{
		{
			FixedPcdGet16(SynapticsCtlrAddress),
			I2C_M_WR,
			sizeof(UINT8),
			(UINT8 *) &Address
		},
		{
			FixedPcdGet16(SynapticsCtlrAddress),
			I2C_M_RD,
			ReadBytes,
			Data
		}
	};

	Transferred = I2cQupProtocol->Transfer(m_Controller, ControllerProbeMsg, 2);
	if (Transferred != 2) Status = EFI_DEVICE_ERROR;
	
exit:
	return Status;
}

EFI_STATUS
EFIAPI
SynaI2cWrite(
	IN UINT8				Address,
	IN UINT8*				Data,
	IN UINT16				WriteBytes
)
{
	EFI_STATUS Status = EFI_SUCCESS;
	int Transferred = 0;

	if (m_Controller == NULL || I2cQupProtocol == NULL)
	{
		Status = EFI_NOT_READY;
		goto exit;
	}

	if (Data == NULL)
	{
		Status = EFI_INVALID_PARAMETER;
		goto exit;
	}

	struct i2c_msg ControllerProbeMsg[] =
	{
		{
			FixedPcdGet16(SynapticsCtlrAddress),
			I2C_M_WR,
			sizeof(UINT8),
			(UINT8 *) &Address
		},
		{
			FixedPcdGet16(SynapticsCtlrAddress),
			I2C_M_WR,
			WriteBytes,
			Data
		}
	};

	Transferred = I2cQupProtocol->Transfer(m_Controller, ControllerProbeMsg, 2);
	if (Transferred != 2) Status = EFI_DEVICE_ERROR;

exit:
	return Status;
}

EFI_STATUS
EFIAPI
SynaInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS Status = EFI_SUCCESS;
	
	UINT8 InfoData[TOUCH_RMI_PAGE_INFO_BYTES] = { 0 };
	UINT8 Address = TOUCH_RMI_PAGE_INFO_ADDRESS;
	UINTN DeviceIndex = 0;
	UINT16 ControllerSlaveAddr = 0;

	UINT8 Page = 0;
	UINT8 PageF12 = 0;
	UINT8 Function = 0;
	UINT32 TouchDataAddress = 0;

	// Device ID
	DeviceIndex = FixedPcdGet32(SynapticsCtlrI2cDevice);
	if (DeviceIndex <= 0)
	{
		DEBUG((EFI_D_ERROR, "Invalid I2C Device \n"));
		Status = EFI_INVALID_PARAMETER;
		goto exit;
	}

	// Controller Slave Address
	ControllerSlaveAddr = FixedPcdGet16(SynapticsCtlrAddress);
	if (ControllerSlaveAddr <= 0)
	{
		DEBUG((EFI_D_ERROR, "Invalid I2C Address \n"));
		Status = EFI_INVALID_PARAMETER;
		goto exit;
	}

	// Locate protocol
	Status = gBS->LocateProtocol(
		&gQcomI2cQupProtocolGuid,
		NULL,
		(VOID *) &I2cQupProtocol
	);

	if (EFI_ERROR(Status))
	{
		DEBUG((EFI_D_ERROR, "Unable to locate I2C protocol \n"));
		goto exit;
	}

	// GPIO Processing
	Status = gBS->LocateProtocol(
		&gQcomGpioTlmmProtocolGuid,
		NULL,
		(VOID *)&GpioTlmmProtocol
	);

	if (EFI_ERROR(Status))
	{
		DEBUG((EFI_D_ERROR, "Unable to locate GPIO protocol \n"));
		goto exit;
	}

	// Power Seq
	Status = SynaPowerUpController();
	if (EFI_ERROR(Status))
	{
		DEBUG((EFI_D_ERROR, "Failed to power on controller \n"));
		goto exit;
	}

	DEBUG((EFI_D_INFO, "Probe Synaptics RMI4 F12 Function     \n"));

	// Locate device
	m_Controller = I2cQupProtocol->GetDevice(DeviceIndex);

	if (m_Controller != NULL && !m_Controller->suspended)
	{
		// Probe device to locate F12 function
		do
		{
			Status = SynaI2cRead(Address, &InfoData[0], TOUCH_RMI_PAGE_INFO_BYTES);
			if (EFI_ERROR(Status))
			{
				DEBUG((EFI_D_ERROR, "I2C Read failed \n"));
				goto exit;
			}

			DEBUG((EFI_D_INFO, "TouchInit: FDT.QueryBase is %d         \n", InfoData[0]));
			DEBUG((EFI_D_INFO, "TouchInit: FDT.commadBase is %d        \n", InfoData[1]));
			DEBUG((EFI_D_INFO, "TouchInit: FDT.controlBase is %d       \n", InfoData[2]));
			DEBUG((EFI_D_INFO, "TouchInit: FDT.Database is %d          \n", InfoData[3]));
			DEBUG((EFI_D_INFO, "TouchInit: FDT.IRQ is %d               \n", InfoData[4]));
			DEBUG((EFI_D_INFO, "TouchInit: FDT.FuncNumber is %d        \n", InfoData[5]));

			if (InfoData[5] == TOUCH_RMI_F12_FUNCTION)
			{
				DEBUG((EFI_D_INFO, "RMI4 F12 Function Found. \n"));
				PageF12 = Page;
				TouchDataAddress = InfoData[3];
				break;
			}

			// Last page. Go out
			if (InfoData[5] == 0 && Address == TOUCH_RMI_PAGE_INFO_ADDRESS)
			{
				break;
			}
			// Switch page
			else if (InfoData[5] == 0 && Address != TOUCH_RMI_PAGE_INFO_ADDRESS)
			{
				DEBUG((EFI_D_INFO, "Switching to next Synaptics RMI4 Page........"));

				Page++;
				Address = TOUCH_RMI_PAGE_INFO_ADDRESS;

				Status = SynaI2cWrite(RMI_CHANGE_PAGE_ADDRESS, &Page, 1);
				if (EFI_ERROR(Status))
				{
					DEBUG((EFI_D_ERROR, "Unable to switch RMI4 page \n"));
					goto exit;
				}

				DEBUG((EFI_D_INFO, "done \n"));
			}
			else
			{
				Function++;
				Address = Address - TOUCH_RMI_PAGE_INFO_BYTES;
			}
		}
		while ((Address > 0) && (Function < TOUCH_RMI_MAX_FUNCTIONS));

		// Determine final result
		if (Function >= TOUCH_RMI_MAX_FUNCTIONS)
		{
			DEBUG((EFI_D_ERROR, "Invalid RMI4 function index \n"));
			Status = EFI_DEVICE_ERROR;
			goto exit;
		}

		if (Address <= 0)
		{
			DEBUG((EFI_D_ERROR, "Invalid RMI4 address \n"));
			Status = EFI_DEVICE_ERROR;
			goto exit;
		}

		// Install protocols

	}
	else
	{
		DEBUG((EFI_D_ERROR, "Device not found, or in invalid state \n"));
		Status = EFI_DEVICE_ERROR;
	}

exit:
	return Status;
}
