#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/MemoryAllocationLib.h>
#include <Library/QcomPlatformI2cQupLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/QcomI2cQup.h>

#include "i2c_qup_p.h"

STATIC LIST_ENTRY mDevices = INITIALIZE_LIST_HEAD_VARIABLE(mDevices);

STATIC struct qup_i2c_dev *qup_i2c_get_dev(UINTN Id)
{
  struct qup_i2c_dev *Device;
  LIST_ENTRY *        Link;

  Device = NULL;
  for (Link = mDevices.ForwardLink; Link != &mDevices;
       Link = Link->ForwardLink) {
    Device = BASE_CR(Link, struct qup_i2c_dev, Link);
    if (Device->id == Id) {
      return Device;
    }
  }

  return NULL;
}

STATIC EFI_STATUS InternalRegisterI2cDevice(struct qup_i2c_dev *Device)
{
  STATIC UINTN NextDeviceId = 1;

  // TODO: check for collision
  if (Device->id == -1)
    Device->id = NextDeviceId++;

  InsertTailList(&mDevices, &Device->Link);

  return EFI_SUCCESS;
}

STATIC EFIAPI EFI_STATUS RegisterI2cDevice(
    struct qup_i2c_dev *device, UINTN clk_freq, UINTN src_clk_rate)
{
  qup_i2c_sec_init(device, clk_freq, src_clk_rate);

  return InternalRegisterI2cDevice(device);
}

STATIC EFIAPI VOID qup_i2c_iterate(qup_i2c_iterate_cb_t cb)
{
  struct qup_i2c_dev *Device;
  LIST_ENTRY *        Link;

  Device = NULL;
  for (Link = mDevices.ForwardLink; Link != &mDevices;
       Link = Link->ForwardLink) {
    Device = BASE_CR(Link, struct qup_i2c_dev, Link);
    cb(Device->id);
  }
}

STATIC QCOM_I2C_QUP_PROTOCOL mI2cQup = {
    qup_i2c_get_dev,
    qup_i2c_xfer,
    qup_i2c_iterate,
};

EFI_STATUS
EFIAPI
I2cQupDxeInitialize(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_HANDLE Handle = NULL;
  EFI_STATUS Status;

  LibQcomPlatformI2cQupAddBusses(RegisterI2cDevice);

  Status = gBS->InstallMultipleProtocolInterfaces(
      &Handle, &gQcomI2cQupProtocolGuid, &mI2cQup, NULL);
  ASSERT_EFI_ERROR(Status);

  return Status;
}

EFI_STATUS
EFIAPI
I2cQupDxeUnload(IN EFI_HANDLE ImageHandle)
{
  struct qup_i2c_dev *Device;

  while (!IsListEmpty(&mDevices)) {
    Device = BASE_CR(mDevices.ForwardLink, struct qup_i2c_dev, Link);
    RemoveEntryList(&Device->Link);

    qup_i2c_deinit(Device);
  }

  return EFI_SUCCESS;
}
