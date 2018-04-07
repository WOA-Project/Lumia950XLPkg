#ifndef _BAM_PRIVATE_H
#define _BAM_PRIVATE_H

void bam_init(struct bam_instance *bam);
void bam_sys_pipe_init(struct bam_instance *bam,
                       uint8_t pipe_num);
int bam_pipe_fifo_init(struct bam_instance *bam,
                       uint8_t pipe_num);
struct cmd_element* bam_add_cmd_element(struct cmd_element *ptr,
                                        uint32_t addr,
                                        uint32_t data,
                                        enum bam_ce_cmd_t cmd_type);
int bam_add_desc(struct bam_instance *bam,
                 unsigned int pipe_num,
                 unsigned char *data_ptr,
                 unsigned int data_len,
                 unsigned flags);
int bam_add_one_desc(struct bam_instance *bam,
                     unsigned int pipe_num,
                     unsigned char*,
                     uint32_t len,
                     uint8_t flags);
void bam_sys_gen_event(struct bam_instance *bam,
                       uint8_t pipe_num,
                       unsigned int num_desc);
int bam_wait_for_interrupt(struct bam_instance *bam,
                           uint8_t pipe_num,
                           enum p_int_type interrupt);
void bam_read_offset_update(struct bam_instance *bam, unsigned int pipe_num);

#endif // _BAM_PRIVATE_H
