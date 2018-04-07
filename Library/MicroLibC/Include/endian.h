#ifndef _ENDIAN_H
#define _ENDIAN_H

//
// all UEFI platforms are little-endian which simplifies this header a lot
//

#define htole16(x)  (x)
#define htole32(x)  (x)
#define htole64(x)  (x)

#define le16toh(x)  htole16(x)
#define le32toh(x)  htole32(x)
#define le64toh(x)  htole64(x)

#endif
