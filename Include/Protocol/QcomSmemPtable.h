#ifndef __QCOM_PROTOCOL_SMEM_PTABLE_H__
#define __QCOM_PROTOCOL_SMEM_PTABLE_H__

#include <Chipset/smem.h>
#include <Library/ptable.h>

#define QCOM_SMEM_PTABLE_PROTOCOL_GUID                                         \
  {                                                                            \
    0x66075564, 0x7aac, 0x4c93,                                                \
    {                                                                          \
      0xb4, 0x62, 0x40, 0xbf, 0xee, 0x1d, 0x59, 0x38                           \
    }                                                                          \
  }

typedef struct _QCOM_SMEM_PTABLE_PROTOCOL QCOM_SMEM_PTABLE_PROTOCOL;

typedef unsigned(EFIAPI *smem_get_apps_flash_start_t)(void);
typedef void(EFIAPI *smem_add_modem_partitions_t)(struct ptable *flash_ptable);

typedef void(EFIAPI *smem_get_ram_ptable_entry_t)(
    ram_partition *, uint32_t entry);
typedef uint32_t(EFIAPI *smem_get_ram_ptable_version_t)(void);
typedef uint32_t(EFIAPI *smem_get_ram_ptable_len_t)(void);
typedef uint32_t(EFIAPI *get_ddr_start_t)(void);

struct _QCOM_SMEM_PTABLE_PROTOCOL {
  smem_get_apps_flash_start_t smem_get_apps_flash_start;
  smem_add_modem_partitions_t smem_add_modem_partitions;

  smem_get_ram_ptable_entry_t   smem_get_ram_ptable_entry;
  smem_get_ram_ptable_version_t smem_get_ram_ptable_version;
  smem_get_ram_ptable_len_t     smem_get_ram_ptable_len;
  get_ddr_start_t               get_ddr_start;
};

extern EFI_GUID gQcomSmemPtableProtocolGuid;

#endif
