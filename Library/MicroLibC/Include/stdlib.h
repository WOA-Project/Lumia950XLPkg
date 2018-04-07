#ifndef _STDLIB_H
#define _STDLIB_H

#include <stdint.h>
#include <Library/MallocLib.h>

int atoi(const char *num);
unsigned int atoui(const char *num);
long atol(const char *num);
unsigned long atoul(const char *num);
unsigned long long atoull(const char *num);
unsigned long strtoul(const char *nptr, char **endptr, int base);

void *bsearch(
    const void *key, const void *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *)
);

#endif
