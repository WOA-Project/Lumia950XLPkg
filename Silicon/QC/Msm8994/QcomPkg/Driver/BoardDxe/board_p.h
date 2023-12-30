#ifndef _SMEM_PRIVATE_H
#define _SMEM_PRIVATE_H

#include <Library/QcomSmemLib.h>

void     board_init(void);
uint32_t board_platform_id(void);
uint32_t board_target_id(void);
uint32_t board_baseband(void);
uint32_t board_hardware_id(void);
uint32_t board_hardware_subtype(void);
uint32_t board_foundry_id(void);
uint32_t board_chip_serial(void);
uint8_t  board_pmic_info(struct board_pmic_data *, uint8_t num_ent);
uint32_t board_pmic_target(uint8_t num_ent);
uint32_t board_soc_version(void);
size_t   board_get_hw_platform_name(void *buf, uint32_t buf_size);
size_t   board_chip_serial_char8(void *buf);

#endif // _SMEM_PRIVATE_H
