#ifndef _STRING_H
#define _STRING_H

#include <stdint.h>
#include <Library/StrLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>

#define strcmp(s1, s2) AsciiStrCmp((s1), (s2))
#define strcasecmp(s1, s2) AsciiStriCmp((s1), (s2))
#define strncmp(s1, s2, n) ((int)AsciiStrnCmp((s1), (s2), (n)))
#define strlen(s) ((size_t)AsciiStrLen((s)))
#define strstr(s1 , s2) AsciiStrStr((s1), (s2))
#define memset(s, c, n) SetMem((s), (UINTN)(n), (UINT8)(c))
#define memcpy(s1, s2, n) CopyMem((s1), (s2), (n))

static inline char *strncpy(char *dest, const char *src, size_t n) {
    RETURN_STATUS Status = AsciiStrCpyS(dest, n, src);
    if (RETURN_ERROR(Status))
        return NULL;

    return dest;
}

static inline char *strncat(char *dest, const char *src, size_t n) {
    RETURN_STATUS Status = AsciiStrnCatS(dest, n, src, n);
    if (RETURN_ERROR(Status))
        return NULL;

    return dest;
}

//
// UNSAFE
//
#define strcpy(dest, src) AsciiStrCpyS((dest), AsciiStrLen((src)), (src))

#endif
