#ifndef _SMEM_PTABLE_PRIVATE_H
#define _SMEM_PTABLE_PRIVATE_H

void     smem_ptable_init(void);
unsigned smem_get_apps_flash_start(void);
void     smem_add_modem_partitions(struct ptable *flash_ptable);

int smem_ram_ptable_init_v1(
    void); /* Used on platforms that use ram ptable v1 */
void     smem_get_ram_ptable_entry(ram_partition *, uint32_t entry);
uint32_t smem_get_ram_ptable_version(void);
uint32_t smem_get_ram_ptable_len(void);
uint32_t get_ddr_start(void);

#endif // _SMEM_PTABLE_PRIVATE_H
