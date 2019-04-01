#ifndef __QCOM_PCIEXPRESS_PROTOCOL_H__
#define __QCOM_PCIEXPRESS_PROTOCOL_H__

#define QCOM_PCIEXPRESS_INIT_GUID                                              \
  {                                                                            \
    0x879baa44, 0xc7ed, 0x1023,                                                \
    {                                                                          \
      0x48, 0x7d, 0xa0, 0x45, 0x5e, 0xbc, 0x20, 0x7e                           \
    }                                                                          \
  }

typedef struct _QCOM_PCI_EXPRESS_INIT_PROTOCOL QCOM_PCI_EXPRESS_INIT_PROTOCOL;

typedef EFI_STATUS(EFIAPI *PCIEXPRESS_INIT_T)(VOID);

struct _QCOM_PCI_EXPRESS_INIT_PROTOCOL {
  PCIEXPRESS_INIT_T InitRootComplex;
};

extern EFI_GUID gQcomMsmPCIExpressInitProtocolGuid;

#endif