#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/QcomGpioTlmm.h>
#include <Protocol/QcomPm8x41.h>
#include <Protocol/QcomSpi.h>
#include <Protocol/QcomSpmi.h>

#include "Bitstream.h"

QCOM_GPIO_TLMM_PROTOCOL *mQcomGpioTlmmProtocol;
QCOM_PM8X41_PROTOCOL *   mQcomPmicProtocol;
QCOM_SPMI_PROTOCOL *     mQcomSpmiProtocol;
QCOM_SPI_QUP_PROTOCOL *  mQcomSpiProtocol;

#define SPI_MOSI 53
#define SPI_MISO 54
#define SPI_CS 55
#define SPI_SCLK 56

EFI_STATUS ReadUc120Register(UINTN Address, unsigned char *Value)
{
  EFI_STATUS Status;

  struct spi_transfer transfer;
  unsigned char       command = (unsigned char)(Address << 3);

  transfer.tx_buf        = &command;
  transfer.rx_buf        = Value;
  transfer.len           = 1;
  transfer.bits_per_word = 8;
  transfer.speed_hz      = 25000000;

  Status = mQcomSpiProtocol->Transfer(&transfer);
  return Status;
}

EFI_STATUS
EFIAPI
LiCE40SpiConfigEntry(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol(
      &gQcomGpioTlmmProtocolGuid, NULL, (VOID **)&mQcomGpioTlmmProtocol);

  ASSERT_EFI_ERROR(Status);

  Status = gBS->LocateProtocol(
      &gQcomPm8x41ProtocolGuid, NULL, (VOID **)&mQcomPmicProtocol);

  ASSERT_EFI_ERROR(Status);

  Status = gBS->LocateProtocol(
      &gQcomSpmiProtocolGuid, NULL, (VOID **)&mQcomSpmiProtocol);

  ASSERT_EFI_ERROR(Status);

  Status = gBS->LocateProtocol(
      &gQcomSpiQupProtocolGuid, NULL, (VOID **)&mQcomSpiProtocol);

  ASSERT_EFI_ERROR(Status);

  // Configure CRESET_B: PMI8994 GPIO4 Out low
  mQcomPmicProtocol->pm8x41_gpio_set_sid(2, 4, 0);

  // Configure PMI8994 GPIO5 Out low
  mQcomPmicProtocol->pm8x41_gpio_set_sid(2, 5, 0);

  // Set PMIC GPIO and LDO
  // PMI8994 LDO19 3.3V On
  struct pm8x41_ldo ldo19 = LDO(PM8x41_LDO19, PLDO_TYPE);
  mQcomPmicProtocol->pm8x41_ldo_set_voltage(&ldo19, 3300000);
  mQcomPmicProtocol->pm8x41_ldo_control(&ldo19, 1);

  // PMI8994 LDO31 1.2V On
  struct pm8x41_ldo ldo31 = LDO(PM8x41_LDO31, PLDO_TYPE);
  mQcomPmicProtocol->pm8x41_ldo_set_voltage(&ldo31, 1200000);
  mQcomPmicProtocol->pm8x41_ldo_control(&ldo31, 1);

  // PMI8994 GPIO 4 2mA output no pull
  struct pm8x41_gpio pmi_gpio4_param = {
      .direction     = PM_GPIO_DIR_OUT,
      .output_buffer = PM_GPIO_OUT_CMOS,
      .out_strength  = PM_GPIO_OUT_DRIVE_MED,
  };

  mQcomPmicProtocol->pm8x41_gpio_config_sid(2, 4, &pmi_gpio4_param);

  // TODO: SPMI write 0xa041, 0xa04a, 0xa046, slave ID 2
  // 0xa041: 0x16
  // 0xa04a: 0x81
  // 0xa046: 1 << 7
  mQcomPmicProtocol->pm8x41_reg_write(0x2a041, 0x16);
  mQcomPmicProtocol->pm8x41_reg_write(0x2a04a, 0x81);
  mQcomPmicProtocol->pm8x41_reg_write(0x2a046, 1 << 7);

  // Configure CDONE TLMM (GPIO 95), 2mA Pull Up for input
  mQcomGpioTlmmProtocol->SetDriveStrength(95, 2);
  mQcomGpioTlmmProtocol->SetFunction(95, 0);
  mQcomGpioTlmmProtocol->SetPull(95, GPIO_PULL_UP);
  mQcomGpioTlmmProtocol->DirectionInput(95);

  // Configure SPI_SS TLMM (GPIO 55) 2mA No Pull, set value to 0
  mQcomGpioTlmmProtocol->SetDriveStrength(55, 2);
  mQcomGpioTlmmProtocol->SetFunction(55, 0);
  mQcomGpioTlmmProtocol->SetPull(55, GPIO_PULL_NONE);
  mQcomGpioTlmmProtocol->DirectionOutput(55, 1);
  mQcomGpioTlmmProtocol->Set(55, 0);

  // Configure CRESET_B: PMI8994 GPIO4 Out low
  mQcomPmicProtocol->pm8x41_gpio_set_sid(2, 4, 0);

  // Wait at least 200ns
  udelay(200);

  // Set PMI8994 GPIO4 Out High, CRESET_B = 1
  mQcomPmicProtocol->pm8x41_gpio_set_sid(2, 4, 1);

  // Wait at least 1200ns
  udelay(1200);

  // Check if CDONE == 0
  if (mQcomGpioTlmmProtocol->Get(95) != 0) {
    DEBUG((EFI_D_ERROR, "CDONE != 0"));
    ASSERT(FALSE);
  }
  else {
    DEBUG((EFI_D_INFO, "CDONE initial check succeeded \n"));
  }

  // Transfer firmware using QUP SPI
  struct spi_transfer transfer;
  UINTN               Transferred = 0;
  void *              CurrentBuf  = gBitstream;
  char                Dummy[7];

  ZeroMem(Dummy, sizeof(Dummy));

  transfer.bits_per_word = 8;
  transfer.speed_hz      = 25000000;

  DEBUG((EFI_D_INFO, "Downloading bitstream at 25MHz \n"));
  while (Transferred < sizeof(gBitstream)) {
    UINTN ActualTransfer = (sizeof(gBitstream) - Transferred >= 0x40)
                               ? 0x40
                               : (sizeof(gBitstream) - Transferred);

    transfer.tx_buf = CurrentBuf;
    transfer.len    = ActualTransfer;

    Status = mQcomSpiProtocol->Transfer(&transfer);
    ASSERT_EFI_ERROR(Status);

    Transferred += ActualTransfer;
    CurrentBuf = CurrentBuf + ActualTransfer;
  }

  // Pull up CS_N
  mQcomGpioTlmmProtocol->SetPull(55, GPIO_PULL_UP);
  mQcomGpioTlmmProtocol->DirectionOutput(55, 1);

  // Transfer dummy
  transfer.len    = sizeof(Dummy);
  transfer.tx_buf = (void *)Dummy;
  Status          = mQcomSpiProtocol->Transfer(&transfer);
  ASSERT_EFI_ERROR(Status);

  if (mQcomGpioTlmmProtocol->Get(95) == 0) {
    DEBUG((EFI_D_ERROR, "CDONE != 1"));
    ASSERT(FALSE);
  }
  else {
    DEBUG((EFI_D_INFO, "CDONE check succeeded!\n"));
  }

  // TODO: Config PM8994 GPIO 13 out (VCONN_OUT_EN) ?
  ASSERT_EFI_ERROR(Status);
  return Status;
}
