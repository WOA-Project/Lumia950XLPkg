#ifndef __QCOM_PROTOCOL_BOARD_H__
#define __QCOM_PROTOCOL_BOARD_H__

#include <Chipset/board.h>

#define QCOM_BOARD_PROTOCOL_GUID                                               \
  {                                                                            \
    0x470d9ff7, 0x23d0, 0x4f25,                                                \
    {                                                                          \
      0x91, 0xaf, 0xe4, 0x5f, 0x3f, 0x6f, 0xe6, 0xb4                           \
    }                                                                          \
  }

typedef struct _QCOM_BOARD_PROTOCOL QCOM_BOARD_PROTOCOL;

typedef uint32_t(EFIAPI *board_platform_id_t)(void);
typedef uint32_t(EFIAPI *board_target_id_t)(void);
typedef uint32_t(EFIAPI *board_baseband_t)(void);
typedef uint32_t(EFIAPI *board_hardware_id_t)(void);
typedef uint32_t(EFIAPI *board_hardware_subtype_t)(void);
typedef uint32_t(EFIAPI *board_foundry_id_t)(void);
typedef uint32_t(EFIAPI *board_chip_serial_t)(void);
typedef uint8_t(EFIAPI *board_pmic_info_t)(
    struct board_pmic_data *, uint8_t num_ent);
typedef uint32_t(EFIAPI *board_pmic_target_t)(uint8_t num_ent);
typedef uint32_t(EFIAPI *board_soc_version_t)(void);
typedef size_t(EFIAPI *board_get_hw_platform_name_t)(
    void *buf, uint32_t buf_size);
typedef size_t(EFIAPI *board_chip_serial_char8_t)(void *buf);

struct _QCOM_BOARD_PROTOCOL {
  board_platform_id_t          board_platform_id;
  board_target_id_t            board_target_id;
  board_baseband_t             board_baseband;
  board_hardware_id_t          board_hardware_id;
  board_hardware_subtype_t     board_hardware_subtype;
  board_foundry_id_t           board_foundry_id;
  board_chip_serial_t          board_chip_serial;
  board_chip_serial_char8_t    board_chip_serial_char8;
  board_pmic_info_t            board_pmic_info;
  board_pmic_target_t          board_pmic_target;
  board_soc_version_t          board_soc_version;
  board_get_hw_platform_name_t board_get_hw_platform_name;
};

extern EFI_GUID gQcomBoardProtocolGuid;

#endif
