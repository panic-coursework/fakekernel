#include "string.h"
#include "panic.h"

size_t strlen (const u8 *s) {
  size_t size;
  for (size = 0; s[size]; ++size) continue;
  return size;
}

size_t strlcpy (u8 * __restrict dest, const u8 * __restrict src, size_t n) {
  kassert(n > 0, "strlcpy: bad n");
  for (size_t i = 0; i < n; ++i) {
    dest[i] = src[i];
    if (src[i] == '\0') {
      return i;
    }
  }
  dest[n - 1] = '\0';
  return n;
}
