#ifndef __LIBRARY_STR_H__
#define __LIBRARY_STR_H__

CHAR8 *strtok(CHAR8 *s, CONST CHAR8 *delim);
CHAR8 *strtok_r(CHAR8 *s, CONST CHAR8 *delim, CHAR8 **last);
UINTN strlcat(CHAR8 *dst, CONST CHAR8 *src, UINTN siz);
UINTN strspn(CHAR8 CONST *s, CHAR8 CONST *accept);
CHAR8 *strchr(CONST CHAR8 *s, INTN c);
CHAR8 *strpbrk(CONST CHAR8 *s, CONST CHAR8 *accept);

#endif
