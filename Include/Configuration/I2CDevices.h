#ifndef _LUMIA_950XL_I2C_DEVICES_H_
#define _LUMIA_950XL_I2C_DEVICES_H_

typedef struct _I2C_DEVICE {
	INTN DeviceId;
	UINT32 QupBase;
	UINT32 QupIrq;
	UINT32 FreqOut;
	UINT32 FreqIn;
	// Device On PIN
	UINT32 EnableDriveStrength;
	GPIO_PULL EnablePull;
	BOOLEAN EnableSetPull;
	UINT32 EnablePinFunction;
	UINT32 EnablePin0;
	UINT32 EnablePin1;
	// Clocks
	UINT32 ClkBlspId;
	UINT32 ClkQupId;
} I2C_DEVICE, *PI2C_DEVICE;

static I2C_DEVICE gI2cDevices[] =
{
	// I2C 1
	{
		1, 0xf9923000, 0x7f, 0x05f5e100, 0x0124f800,
		2, GPIO_PULL_NONE, TRUE,
		3, 2, 3,
		1, 1
	},
	// I2C 2
	{
		2, 0xf9924000, 0x80, 0x05f5e100, 0x0124f800,
		2, GPIO_PULL_NONE, TRUE,
		3, 6, 7,
		1, 2
	},
	// I2C 6
	{
		6, 0xf9928000, 0x84, 0x05f5e100, 0x0124f800,
		2, GPIO_PULL_NONE, TRUE,
		3, 27, 28,
		1, 6
	},
	// I2C 7
	{
		7, 0xf9963000, 0x85, 0x05f5e100, 0x0124f800,
		2, GPIO_PULL_NONE, TRUE,
		3, 43, 44,
		2, 1
	},
	// Terminator
	{ }
};

#endif