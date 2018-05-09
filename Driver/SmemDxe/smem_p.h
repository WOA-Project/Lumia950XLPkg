#ifndef _SMEM_PRIVATE_H
#define _SMEM_PRIVATE_H

/* Note: buf MUST be 4byte aligned, and max_len MUST be a multiple of 8. */
unsigned smem_read_alloc_entry(smem_mem_type_t type, void *buf, int max_len);
unsigned smem_read_alloc_entry_offset(smem_mem_type_t type, void *buf, int len, int offset);
void* smem_get_alloc_entry(smem_mem_type_t type, uint32_t* size);

#endif // _SMEM_PRIVATE_H
