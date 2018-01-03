/** @file endian.h
 *  @brief Simple utilities for endianness conversion.
 */
#ifndef MISC_ENDIAN_H_
#define MISC_ENDIAN_H_

#include <stdint.h>

namespace endian {

/// Types of endianness.
enum ByteOrder { BO_UNKNOWN = 0, BO_LITTLE_ENDIAN=1234, BO_BIG_ENDIAN=4321 };

/// Convert to native byte order.
template <class T>
T toNative(T x, ByteOrder origOrder)
{
  // the compiler might actually be able to optimize this quite nicely,
  // without the need for template specializations
  T res = 0;
  int n = sizeof(T);
  const unsigned char* p = reinterpret_cast<const unsigned char*>(&x);
  if (origOrder == BO_LITTLE_ENDIAN) {
    p += n - 1;
    for ( ; n > 0; --n, --p)
      res = (res << 8) + T(*p);
  } else if (origOrder == BO_BIG_ENDIAN) {
    for ( ; n > 0; --n, ++p)
      res = (res << 8) + T(*p);
  }

  return res;
}

/// Convert from native byte order.
template <class T>
T fromNative(T x, ByteOrder destOrder)
{
  T res = 0;
  int n = sizeof(T);
  unsigned char* p = reinterpret_cast<unsigned char*>(&res);
  if (destOrder == BO_LITTLE_ENDIAN) {
    for ( ; n > 0; --n, ++p) {
      *p = (x & 0xFF);
      x >>= 8;
    }
  } else if (destOrder == BO_BIG_ENDIAN) {
    p += n - 1;
    for ( ; n > 0; --n, --p) {
      *p = (x & 0xFF);
      x >>= 8;
    }
  }

  return res;
}

} // namespace endian

#endif
