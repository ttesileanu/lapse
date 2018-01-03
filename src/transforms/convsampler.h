/** @file convsampler.h
 *  @brief A sampler that uses a convolution with a matrix.
 */
#ifndef TRANSFORMS_CONVSAMPLER_H_
#define TRANSFORMS_CONVSAMPLER_H_

#include <utility>
#include <vector>

#include <cmath>

#include "sampler.h"

/** @brief Sampler that performs 2d or 1d convolutions.
 *
 *  The function that the convolution is performed with is assumed to be a
 *  product, fct(x, y) = f1(x)*f2(y), and each term in the product is read from
 *  a look-up table (for efficiency).
 *
 *  The sampler can apply the convolution in both directions, or only
 *  horizontally or vertically, in which case the other direction just uses
 *  a nearest-pixel approximation.
 */
template <class T>
class ConvolutionSampler : public BaseSampler<T> {
 public:
  typedef typename BaseSampler<T>::Direction Direction;

  /** @brief Constructor.
   *
   *  The default sampler just returns the nearest pixel value.
   */
  ConvolutionSampler() : lutX_(1, 1), lutY_(1, 1), sizeX_(0.5),
      sizeY_(0.5) {}

  /** @brief Sample the picture.
   *
   *  The @a dir parameter chooses whether sampling is done in both directions,
   *  or only horizontally or vertically.
   *
   *  The filter image size is scaled by @a scalex, @a scaley with respect to
   *  the sizes set/retrieved by @a setSize / @a getSize.
   *
   */
  virtual void get(const GenericImage<T>& image, float x, float y, T* where,
      Direction dir = BaseSampler<T>::BOTH, float scalex = 1, float scaley = 1)
      const;

  /// Update the horizontal look-up table.
  void setLutX(const std::vector<float>& newlut) { lutX_ = newlut; }
  /// Update the vertical look-up table.
  void setLutY(const std::vector<float>& newlut) { lutY_ = newlut; }
  /// Update both look-up tables with the same vector.
  void setLuts(const std::vector<float>& newlut) { lutX_ = lutY_ = newlut; }

  /// Look at the horizontal look-up table.
  const std::vector<float>& getLutX() const { return lutX_; }
  /// Look at the vertical look-up table.
  const std::vector<float>& getLutY() const { return lutY_; }

  /// Set the filter image size.
  void setSize(float x, float y) { sizeX_ = x; sizeY_ = y; }

  /// Get the filter image size.
  std::pair<float, float> getSize() const
    { return std::make_pair(sizeX_, sizeY_); }
  /// Get the filter horizontal image size.
  float getSizeX() const { return sizeX_; }
  /// Get the filter vertical image size.
  float getSizeY() const { return sizeY_; }

 protected:
  /// Get using a product of the filter with itself.
  void getProduct_(const GenericImage<T>& image, float x, float y, T*,
    float scalex, float scaley) const;
  
  /// Get using the filter only horizontally.
  void getX_(const GenericImage<T>& image, float x, float y, T* where,
    float scalex) const;
  /// Get using the filter only vertically.
  void getY_(const GenericImage<T>& image, float x, float y, T* where,
    float scaley) const;

  /** @brief Filter look-up table for the horizontal direction.
   *
   *  Position @a x + @a dx in image space is mapped to
   *  (@a dx + @a sizeX_)*@a lutX_.size() / (2*@a sizeX_)
   *  in this vector.
   */
  std::vector<float>   lutX_;
  /** @brief Filter look-up table for the vertical direction.
   *
   *  Position @a y + @a dy in image space is mapped to
   *  (@a dy + @a sizeY_)*@a lutY_.size() / (2*@a sizeY_)
   *  in this vector.
   */
  std::vector<float>   lutY_;
  /** @brief Horizontal "radius" of the filter on the image.
   *
   *  The filter goes from @a x - @a sizeX_ (inclusive) to @a x + @a sizeX_
   *  (exclusive).
   */
  float                 sizeX_;
  /** @brief Vertical "radius" of the filter on the image.
   *
   *  The filter goes from @a y - @a sizeY_ (inclusive) to @a y + @a sizeY_
   *  (exclusive).
   */
  float                 sizeY_;
};

// inline functions
template <class T>
inline void ConvolutionSampler<T>::get(const GenericImage<T>& image, float x,
    float y, T* where, Direction dir, float scalex, float scaley) const
{
  switch (dir) {
    case BaseSampler<T>::BOTH:
      getProduct_(image, x, y, where, scalex, scaley);
      return;
    case BaseSampler<T>::HORIZONTAL:
      getX_(image, x, y, where, scalex);
      return;
    case BaseSampler<T>::VERTICAL:
      getY_(image, x, y, where, scaley);
      return;
    default:;
  }
  const size_t comps = image.getChannelCount();
  const T* p = image(x, y);
  for (size_t comp = 0; comp < comps; ++comp) {
    where[comp] = p[comp];
  }
}

#endif
