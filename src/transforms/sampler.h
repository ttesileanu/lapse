/** @file sampler.h
 *  @brief Infrastructure for using interpolation to access image values at
 *  non-integer positions.
 */
#ifndef TRANSFORMS_SAMPLER_H_
#define TRANSFORMS_SAMPLER_H_

#include "image/image.h"

/// Abstract class, defining the sampler interface.
template <class T>
class BaseSampler {
 public:
  /// Direction(s) in which to apply the interpolation.
  enum Direction { NONE = 0, HORIZONTAL, VERTICAL, BOTH };

  /// Virtual destructor -- for inheritance.
  virtual ~BaseSampler() {}

  /** @brief Sample the picture at the given point.
   *
   *  This places the output in @a where. The @a dir parameter allows the filter
   *  to be applied either to both directions, or only horizontally or
   *  vertically.
   *
   *  The filter image size is scaled by @a scalex, @a scaley with respect to
   *  the sizes set/retrieved by @a setSize / @a getSize.
   */
  virtual void get(const GenericImage<T>& image, float x, float y, T* where,
      Direction dir = BaseSampler<T>::BOTH, float scalex = 1, float scaley = 1)
      const = 0;
};

#endif
