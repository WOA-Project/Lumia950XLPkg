#ifndef __QCOM_PROTOCOL_BAM_H__
#define __QCOM_PROTOCOL_BAM_H__

#include <Chipset/bam.h>

#define QCOM_BAM_PROTOCOL_GUID \
  { 0xacdd545a, 0xf1f6, 0x4272, { 0x81, 0xc5, 0x04, 0x93, 0xe3, 0x58, 0x05, 0x32 } }

typedef struct _QCOM_BAM_PROTOCOL   QCOM_BAM_PROTOCOL;

typedef void (EFIAPI *bam_init_t)(struct bam_instance *bam);
typedef void (EFIAPI *bam_sys_pipe_init_t)(struct bam_instance *bam, uint8_t pipe_num);
typedef int  (EFIAPI *bam_pipe_fifo_init_t)(struct bam_instance *bam, uint8_t pipe_num);
typedef struct cmd_element* (EFIAPI *bam_add_cmd_element_t)(struct cmd_element *ptr, uint32_t addr, uint32_t data, enum bam_ce_cmd_t cmd_type);
typedef int  (EFIAPI *bam_add_desc_t)(struct bam_instance *bam, unsigned int pipe_num, unsigned char *data_ptr, unsigned int data_len, unsigned flags);
typedef int  (EFIAPI *bam_add_one_desc_t)(struct bam_instance *bam, unsigned int pipe_num, unsigned char*, uint32_t len, uint8_t flags);
typedef void (EFIAPI *bam_sys_gen_event_t)(struct bam_instance *bam, uint8_t pipe_num, unsigned int num_desc);
typedef int  (EFIAPI *bam_wait_for_interrupt_t)(struct bam_instance *bam, uint8_t pipe_num, enum p_int_type interrupt);
typedef void (EFIAPI *bam_read_offset_update_t)(struct bam_instance *bam, unsigned int pipe_num);

struct _QCOM_BAM_PROTOCOL {
  bam_init_t                bam_init;
  bam_sys_pipe_init_t       bam_sys_pipe_init;
  bam_pipe_fifo_init_t      bam_pipe_fifo_init;
  bam_add_cmd_element_t     bam_add_cmd_element;
  bam_add_desc_t            bam_add_desc;
  bam_add_one_desc_t        bam_add_one_desc;
  bam_sys_gen_event_t       bam_sys_gen_event;
  bam_wait_for_interrupt_t  bam_wait_for_interrupt;
  bam_read_offset_update_t  bam_read_offset_update;
};

extern EFI_GUID gQcomBamProtocolGuid;

#endif
