#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/QcomGpioTlmm.h>
#include <Protocol/QcomPm8x41.h>
#include <Protocol/QcomSpmi.h>

#include "Bitstream.h"

QCOM_GPIO_TLMM_PROTOCOL *mQcomGpioTlmmProtocol;
QCOM_PM8X41_PROTOCOL *   mQcomPmicProtocol;
QCOM_SPMI_PROTOCOL *     mQcomSpmiProtocol;

#define SPI_MOSI 53
#define SPI_MISO 54
#define SPI_CS 55
#define SPI_SCLK 56

#define TLMM_CONFIGURE_SPI_OUT(x)                                              \
  mQcomGpioTlmmProtocol->SetFunction((x), 0);                                  \
  mQcomGpioTlmmProtocol->SetDriveStrength((x), 2);                             \
  mQcomGpioTlmmProtocol->SetPull((x), GPIO_PULL_NONE);                         \
  mQcomGpioTlmmProtocol->DirectionOutput((x), 0)

// spi_bit_bang_write: Write into SPI pipe using bitbang
void spi_bit_bang_write(UINT8 data)
{
  int i;

  // send bits 7..0
  for (i = 0; i < 8; i++) {
    // consider leftmost bit
    // set line high if bit is 1, low if bit is 0
    if (data & 0x80) {
      mQcomGpioTlmmProtocol->Set(SPI_MOSI, 1);
    }
    else {
      mQcomGpioTlmmProtocol->Set(SPI_MOSI, 0);
    }
    udelay(1);

    // pulse clock to indicate that bit value should be read
    mQcomGpioTlmmProtocol->Set(SPI_SCLK, 0);
    udelay(100);
    mQcomGpioTlmmProtocol->Set(SPI_SCLK, 1);

    // shift byte left so next bit will be leftmost
    data <<= 1;
  }
}

// The actual write function
int truly_spi_write(UINT8 *data, int num)
{
  int i;
  mQcomGpioTlmmProtocol->Set(SPI_CS, 0); /* cs low */
  udelay(100);

  for (i = 0; i < num; i++) {
    spi_bit_bang_write(data[i]);
  }

  mQcomGpioTlmmProtocol->Set(SPI_CS, 1); /* cs high */
  udelay(100);

  // H/W needs delay for gpio's to turn high
  udelay(10);
  return 0;
}

EFI_STATUS
EFIAPI
LiCE40SpiConfigEntry(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  // It may be necesary to run some init first
  EFI_STATUS Status;

  Status = gBS->LocateProtocol(
      &gQcomGpioTlmmProtocolGuid, NULL, (VOID **)&mQcomGpioTlmmProtocol);

  if (EFI_ERROR(Status)) {
    goto exit;
  }

  Status = gBS->LocateProtocol(
      &gQcomPm8x41ProtocolGuid, NULL, (VOID **)&mQcomPmicProtocol);

  if (EFI_ERROR(Status)) {
    goto exit;
  }

  Status = gBS->LocateProtocol(
      &gQcomSpmiProtocolGuid, NULL, (VOID **)&mQcomSpmiProtocol);

  if (EFI_ERROR(Status)) {
    goto exit;
  }

  // Configure SPI output pins（2mA, no pull, out)
  TLMM_CONFIGURE_SPI_OUT(SPI_MOSI);
  TLMM_CONFIGURE_SPI_OUT(SPI_CS);
  TLMM_CONFIGURE_SPI_OUT(SPI_SCLK);

  // Configure CRESET_B: PMI8994 GPIO4 Out low
  mQcomPmicProtocol->pm8x41_gpio_set_sid(2, 4, 0);

  // Configure PMI8994 GPIO5 Out low
  mQcomPmicProtocol->pm8x41_gpio_set_sid(2, 5, 0);

  // Set PMIC GPIO and LDO
  // PMI8994 LDO18 3.3V On
  struct pm8x41_ldo ldo18 = LDO(PM8x41_LDO18, PLDO_TYPE);
  mQcomPmicProtocol->pm8x41_ldo_set_voltage(&ldo18, 3300000);
  mQcomPmicProtocol->pm8x41_ldo_control(&ldo18, 1);

  // PMI8994 LDO30 1.2V On
  struct pm8x41_ldo ldo30 = LDO(PM8x41_LDO30, PLDO_TYPE);
  mQcomPmicProtocol->pm8x41_ldo_set_voltage(&ldo30, 1200000);
  mQcomPmicProtocol->pm8x41_ldo_control(&ldo30, 1);

  // PMI8994 GPIO 4 2mA output no pull
  struct pm8x41_gpio pmi_gpio4_param = {
      .direction     = PM_GPIO_DIR_OUT,
      .output_buffer = PM_GPIO_OUT_CMOS,
      .out_strength  = PM_GPIO_OUT_DRIVE_MED,
  };

  mQcomPmicProtocol->pm8x41_gpio_config_sid(2, 4, &pmi_gpio4_param);

  // SPMI write 0xa041, 0xa04a, 0xa046, slave ID 2
  // 0xa041: 0x16
  // 0xa04a: 0x81
  // 0xa046: 1 << 7
  struct pmic_arb_cmd   spmi_cmd;
  struct pmic_arb_param spmi_param;
  UINT8                 buf;

  spmi_cmd.address  = 0xa041 >> 8;
  spmi_cmd.offset   = (UINT8)0xa041;
  spmi_cmd.byte_cnt = 1;
  spmi_cmd.slave_id = 2;

  buf               = 0x16;
  spmi_param.buffer = &buf;
  spmi_param.size   = 1;
  mQcomSpmiProtocol->pmic_arb_write_cmd(&spmi_cmd, &spmi_param);

  spmi_cmd.address = 0xa04a >> 8;
  spmi_cmd.offset  = (UINT8)0xa04a;
  buf              = 0x81;
  mQcomSpmiProtocol->pmic_arb_write_cmd(&spmi_cmd, &spmi_param);

  spmi_cmd.address = 0xa046 >> 8;
  spmi_cmd.offset  = (UINT8)0xa046;
  buf              = 1 << 7;
  mQcomSpmiProtocol->pmic_arb_write_cmd(&spmi_cmd, &spmi_param);

  // Configure CRESET_B TLMM (GPIO 95), 2mA Pull Up for input
  mQcomGpioTlmmProtocol->SetDriveStrength(95, 2);
  mQcomGpioTlmmProtocol->SetFunction(95, 0);
  mQcomGpioTlmmProtocol->SetPull(95, GPIO_PULL_UP);
  mQcomGpioTlmmProtocol->DirectionInput(95);

  // Configure SPI_SS TLMM (GPIO 55) 2mA No Pull, set value to 0
  mQcomGpioTlmmProtocol->SetDriveStrength(55, 2);
  mQcomGpioTlmmProtocol->SetFunction(55, 0);
  mQcomGpioTlmmProtocol->SetPull(55, GPIO_PULL_NONE);
  mQcomGpioTlmmProtocol->DirectionOutput(55, 0);

  // Configure CRESET_B: PMI8994 GPIO4 Out low
  mQcomPmicProtocol->pm8x41_gpio_set_sid(2, 4, 0);

  // Set SPI_SCK to 1
  mQcomGpioTlmmProtocol->Set(SPI_SCLK, 1);

  // Wait at least 200ns
  udelay(200);

  // Set PMI8994 GPIO4 Out High, CRESET_B = 1
  mQcomPmicProtocol->pm8x41_gpio_set_sid(2, 4, 1);

  // Wait at least 1200ns
  udelay(1200);

  // Set SPI_SS (GPIO 55) set to high, send 8 dummy clocks
  mQcomGpioTlmmProtocol->Set(55, 1);
  for (UINTN i = 0; i < 8; i++) {
    mQcomGpioTlmmProtocol->Set(SPI_SCLK, 0);
    udelay(100);
    mQcomGpioTlmmProtocol->Set(SPI_SCLK, 1);
  }

  // Transfer bitstream
  truly_spi_write(gBitstream, sizeof(gBitstream));

  // Wait 100 clk cycles
  for (UINTN i = 0; i < 8; i++) {
    // idk
    udelay(100);
  }

  // Check CDONE (INT_N) from TLMM GPIO 95
  if (mQcomGpioTlmmProtocol->Get(95) == 0) {
    DEBUG((EFI_D_ERROR, "CDONE != 1"));
    ASSERT(FALSE);
  }

  // Send additional 49 dummy bits and 49 SCK (56) clk cycles
  unsigned char dummy[49];
  ZeroMem(dummy, sizeof(dummy));
  truly_spi_write(dummy, sizeof(dummy));
  for (UINTN i = 0; i < 49; i++) {
    mQcomGpioTlmmProtocol->Set(SPI_SCLK, 0);
    udelay(100);
    mQcomGpioTlmmProtocol->Set(SPI_SCLK, 1);
  }

  // Config PMI8994 GPIO 13 out (VCONN_OUT_EN)
  mQcomPmicProtocol->pm8x41_gpio_set_sid(2, 13, 0);

exit:
  return Status;
}