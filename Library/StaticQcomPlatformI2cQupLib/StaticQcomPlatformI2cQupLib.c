#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/QcomClockLib.h>
#include <Library/QcomGpioTlmmLib.h>
#include <Library/QcomPlatformI2cQupLib.h>

#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Configuration/I2CDevices.h>

STATIC
EFI_STATUS
EFIAPI
LibQcomPlatformI2cQupBlspClockConfig(UINT32 QupBase)
{
  int         discard = 0;
  uint8_t     ret     = 0;
  char        clk_name[64];
  struct clk *qup_clk;

  discard =
      snprintf(clk_name, sizeof(clk_name), "qup%08x_ahb_iface_clk", QupBase);
  ret = gClock->clk_get_set_enable(clk_name, 0, 1);
  if (ret) {
    DEBUG((DEBUG_ERROR, "Failed to enable %a clock\n", clk_name));
    return EFI_DEVICE_ERROR;
  }

  discard =
      snprintf(clk_name, sizeof(clk_name), "gcc_qup%08x_i2c_apps_clk", QupBase);
  qup_clk = gClock->clk_get(clk_name);
  if (!qup_clk) {
    DEBUG((DEBUG_ERROR, "Failed to get %a\n", clk_name));
    return EFI_NOT_FOUND;
  }

  ret = gClock->clk_enable(qup_clk);
  if (ret) {
    DEBUG((DEBUG_ERROR, "Failed to enable %a\n", clk_name));
    return EFI_DEVICE_ERROR;
  }

  discard = 0;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
LibQcomPlatformI2cQupAddBusses(REGISTER_I2C_DEVICE RegisterI2cDevice)
{
  EFI_STATUS          Status;
  PI2C_DEVICE         I2cDevice = gI2cDevices;
  struct qup_i2c_dev *dev;

  while (I2cDevice->DeviceId > 0) {
    // Set Pins
    gGpioTlmm->SetFunction(I2cDevice->SdaPin, I2cDevice->PinFunction);
    gGpioTlmm->SetDriveStrength(I2cDevice->SdaPin, I2cDevice->DriveStrength);
    if (I2cDevice->EnableSetPull) {
      gGpioTlmm->SetPull(I2cDevice->SdaPin, I2cDevice->DriveStrength);
    }

    gGpioTlmm->SetFunction(I2cDevice->SclPin, I2cDevice->PinFunction);
    gGpioTlmm->SetDriveStrength(I2cDevice->SclPin, I2cDevice->DriveStrength);
    if (I2cDevice->EnableSetPull) {
      gGpioTlmm->SetPull(I2cDevice->SclPin, I2cDevice->DriveStrength);
    }

    // Set Clock
    Status = LibQcomPlatformI2cQupBlspClockConfig(I2cDevice->QupBase);
    if (EFI_ERROR(Status)) {
      DEBUG(
          (EFI_D_ERROR, "Can't configure clocks for I2C device %u \n",
           I2cDevice->DeviceId));
      goto nextDevice;
    }

    // Register device
    dev = AllocateZeroPool(sizeof(struct qup_i2c_dev));
    if (!dev) {
      DEBUG((EFI_D_ERROR, "Can't allocate memory for I2C device\n"));
      goto nextDevice;
    }

    dev->id       = I2cDevice->DeviceId;
    dev->qup_irq  = PcdGet64(PcdGicSpiStart) + I2cDevice->QupIrq;
    dev->qup_base = I2cDevice->QupBase;

    dev->device_type = I2C_QUP_DEVICE_TYPE_BLSP;
    Status = RegisterI2cDevice(dev, I2cDevice->FreqOut, I2cDevice->FreqIn);
    if (EFI_ERROR(Status)) {
      DEBUG(
          (EFI_D_ERROR, "Failed register I2C device 0x%08x with ID %u\n",
           dev->qup_base, dev->id));
      goto nextDevice;
    }

    DEBUG(
        (EFI_D_INFO, "Registered i2c device 0x%08x with ID %u\n", dev->qup_base,
         dev->id));

  nextDevice:
    I2cDevice++;
  }

  return EFI_SUCCESS;
}

RETURN_STATUS
EFIAPI
PlatformI2cQupLibConstructor(VOID) { return EFI_SUCCESS; }