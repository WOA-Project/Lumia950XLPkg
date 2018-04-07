#include <Library/PrintLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#include <stdio.h>

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    CHAR8 localfmt[100];
    UINTN i;
    UINTN fmtlen = AsciiStrSize(format);

    // copy fmt to our buffer
    CopyMem(localfmt, format, fmtlen); 
    localfmt[fmtlen-1] = 0;

    // fix format
    for(i=1; localfmt[i]; i++) {
        if (localfmt[i-1]=='%' && localfmt[i]=='s')
            localfmt[i] = 'a';
    }

    return AsciiVSPrint(str, size, localfmt, ap);
}

int snprintf(char *str, size_t size, const char *format, ...) {
    int err;

    VA_LIST ap;
    VA_START(ap, format);
    err = vsnprintf(str, size, format, ap);
    VA_END(ap);

    return err;
}

int vprintf(const char *format, va_list ap) {
    CHAR8 buf[100];

    vsnprintf(buf, ARRAY_SIZE(buf), format, ap);
    DEBUG((DEBUG_ERROR, "%a", buf));
    return 0;
}

int printf(const char *format, ...) {
    int err;

    VA_LIST ap;
    VA_START(ap, format);
    err = vprintf(format, ap);
    VA_END(ap);

    return err;
}
