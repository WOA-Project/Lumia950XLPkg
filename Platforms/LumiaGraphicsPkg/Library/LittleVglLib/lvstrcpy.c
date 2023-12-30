#include "lvcompat.h"

CHAR8 *EFIAPI AsciiStrCpy(OUT CHAR8 *Destination, IN CONST CHAR8 *Source)
{
  CHAR8 *ReturnValue;
  UINTN  Length;
  //
  // Destination cannot be NULL
  //
  ASSERT(Destination != NULL);

  //
  // Destination and source cannot overlap
  //
  Length = AsciiStrLen(Source);
  if (Source > Destination) {
    ASSERT((UINTN)((Destination + Length + 1) < Source));
  }
  else {
    ASSERT((UINTN)((Source + Length + 1) < Destination));
  }

  ReturnValue = Destination;
  while (*Source != 0) {
    *(Destination++) = *(Source++);
  }
  *Destination = 0;
  return ReturnValue;
}