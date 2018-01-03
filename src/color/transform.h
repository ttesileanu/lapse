/** @file transform.h
 *  @brief A C++ wrapper for LCMS2 transforms.
 */
#ifndef COLOR_TRANSFORM_H_
#define COLOR_TRANSFORM_H_

#include <lcms2.h>

#include "misc/refcount.h"

inline void colorTransformDeleter(cmsHTRANSFORM h)
{
  if (h) cmsDeleteTransform(h); 
}

/** @brief An LCMS transform.
 *
 *  This is a reference-counted wrapper around an LCMS2 transform handle. This
 *  allows transforms to be copied freely, and released when all the copies are
 *  destroyed. Do note that only the handle is actually copied, which means
 *  that all copies of a @a ColorTransform will refer to the same LCMS2
 *  transform.
 */
class ColorTransform : public RefCount<cmsHTRANSFORM, colorTransformDeleter> {
 public:
  /** @brief Constructor.
   *
   *  Note that, if provided, this takes ownership of the handle -- it will be
   *  deleted when this object and all its copies are destroyed.
   */
  explicit ColorTransform(cmsHTRANSFORM handle = 0)
      : RefCount<cmsHTRANSFORM, colorTransformDeleter>(handle) {}

  /// Get underlying C handle.
  // XXX this breaks encapsulation, since the caller can then employ the C
  // interface to alter the profile!
  cmsHTRANSFORM getHandle() const { return handle_; }

  /// Apply the transform.
  template <class Iterator1, class Iterator2>
  void apply(Iterator1 begin, Iterator2 result, size_t npixels) {
    cmsDoTransform(handle_, (const void*)(&(*begin)), (void*)(&(*result)), 
      npixels);
  }
};

#endif
