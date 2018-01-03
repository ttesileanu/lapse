/** @file profile.h
 *  @brief A C++ wrapper for LCMS2 profiles.
 */
#ifndef COLOR_PROFILE_H_
#define COLOR_PROFILE_H_

#include <lcms2.h>

#include "misc/refcount.h"

inline void colorProfileCloser(cmsHPROFILE h)
{
  if (h) cmsCloseProfile(h);
}

/** @brief An LCMS profile.
 *
 *  This is a reference-counted wrapper around an LCMS2 profile handle. This
 *  allows profiles to be copied freely, and the profile will be released when
 *  all the copies are destroyed. Do note that only the handle is actually
 *  copied, which means that all copies of a @a ColorProfile will refer to the
 *  same LCMS2 profile.
 */
class ColorProfile : public RefCount<cmsHPROFILE, colorProfileCloser> {
 public:
  /** @brief Constructor based on C handle.
   *
   *  Note that, if provided, this takes ownership of the handle -- it will be
   *  deleted when this object and all its copies are destroyed.
   */
  explicit ColorProfile(cmsHPROFILE handle = 0)
      : RefCount<cmsHPROFILE, colorProfileCloser>(handle) {}

  /// Get underlying C handle.
  // XXX this breaks encapsulation, since the caller can then employ the C
  // interface to alter the profile! we do, however, need this for a variety
  // of functions...
  cmsHPROFILE getHandle() const { return handle_; }

  /// Get size of profile, in bytes.
  size_t getSize() const {
    cmsUInt32Number size = 0;
    cmsSaveProfileToMem(handle_, 0, &size);
    
    return size;
  };
};

#endif
