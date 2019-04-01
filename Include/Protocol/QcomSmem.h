#ifndef __QCOM_PROTOCOL_SMEM_H__
#define __QCOM_PROTOCOL_SMEM_H__

#include <Chipset/smem.h>

#define QCOM_SMEM_PROTOCOL_GUID                                                \
  {                                                                            \
    0xe738ff9d, 0x020f, 0x4ecd,                                                \
    {                                                                          \
      0x87, 0xfc, 0xd5, 0x82, 0x50, 0x75, 0xaa, 0xcd                           \
    }                                                                          \
  }

typedef struct _QCOM_SMEM_PROTOCOL QCOM_SMEM_PROTOCOL;

typedef unsigned(EFIAPI *smem_read_alloc_entry_t)(
    smem_mem_type_t type, void *buf, int len);
typedef void *(EFIAPI *smem_get_alloc_entry_t)(
    smem_mem_type_t type, uint32_t *size);
typedef unsigned(EFIAPI *smem_read_alloc_entry_offset_t)(
    smem_mem_type_t type, void *buf, int len, int offset);

struct _QCOM_SMEM_PROTOCOL {
  smem_read_alloc_entry_t        smem_read_alloc_entry;
  smem_read_alloc_entry_offset_t smem_read_alloc_entry_offset;
  smem_get_alloc_entry_t         smem_get_alloc_entry;
};

extern EFI_GUID gQcomSmemProtocolGuid;

#endif