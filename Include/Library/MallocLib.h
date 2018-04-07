#ifndef __LIBRARY_MALLOC_H
#define __LIBRARY_MALLOC_H

VOID *malloc(UINTN size);
VOID *memalign(UINTN boundary, UINTN size);
VOID *calloc(UINTN count, UINTN size);
VOID free(VOID *ptr);

#endif
