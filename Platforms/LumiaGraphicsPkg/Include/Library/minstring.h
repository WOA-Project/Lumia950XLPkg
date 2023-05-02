#ifndef __MIN_STRING_H__
#define __MIN_STRING_H__

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>

#define strcmp(s1, s2) ((int)AsciiStrCmp((s1), (s2)))
#define strncmp(s1, s2, n) ((int)AsciiStrnCmp((s1), (s2), (n)))
#define strlen(s) ((size_t)AsciiStrLen((s)))
#define strlcpy(dst, src, n) AsciiStrCpyS((dst), (n), (src))
#define strstr(s1, s2) AsciiStrStr((s1), (s2))
#define memset(s, c, n) SetMem((s), (UINTN)(n), (UINT8)(c))
#define memcpy(s1, s2, n) CopyMem((s1), (s2), (n))
#define memmove(s1, s2, n) CopyMem((s1), (s2), (n))
#define memcmp(s1, s2, n) ((int)CompareMem((s1), (s2), (n)))

#endif
