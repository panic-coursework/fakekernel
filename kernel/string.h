#pragma once

#include "type.h"

size_t strlen (const u8 *s);
// Writes at most n bytes to dest (copies at most n-1 bytes).
// Returns length of string tried to create.
// See strlcpy(3).
size_t strlcpy (u8 * __restrict dest, const u8 * __restrict src, size_t n);
