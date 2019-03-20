#ifndef __QCOM_PROTOCOL_SSBI_H__
#define __QCOM_PROTOCOL_SSBI_H__

#define QCOM_SSBI_PROTOCOL_GUID                                                \
  {                                                                            \
    0x659CFADB, 0xF14F, 0x4088,                                                \
    {                                                                          \
      0x9F, 0x64, 0xB0, 0xE8, 0xA6, 0xAB, 0x66, 0x81                           \
    }                                                                          \
  }

typedef struct _QCOM_SSBI_PROTOCOL QCOM_SSBI_PROTOCOL;

typedef int(EFIAPI *QCOM_SSBI_RW_FUNCTION)(
    unsigned char *buffer, unsigned short length, unsigned short slave_addr);

struct _QCOM_SSBI_PROTOCOL {
  QCOM_SSBI_RW_FUNCTION i2c_ssbi_read_bytes;
  QCOM_SSBI_RW_FUNCTION i2c_ssbi_write_bytes;
  QCOM_SSBI_RW_FUNCTION pa1_ssbi2_read_bytes;
  QCOM_SSBI_RW_FUNCTION pa1_ssbi2_write_bytes;
  QCOM_SSBI_RW_FUNCTION pa2_ssbi2_read_bytes;
  QCOM_SSBI_RW_FUNCTION pa2_ssbi2_write_bytes;
};

extern EFI_GUID gQcomSsbiProtocolGuid;

#endif
