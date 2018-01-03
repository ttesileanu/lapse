/** @file lanczossampler.h
 *  @brief Lanczos filter.
 */
#ifndef TRANSFORMS_LANCZOSSAMPLER_H_
#define TRANSFORMS_LANCZOSSAMPLER_H_

#include <vector>

#include "sampler.h"
#include "convsampler.h"

namespace detail_lanczos {

  /// Calculate the look-up table.
  extern std::vector<float> makeLut(float size, unsigned res);

} // namespace detail_lanczos

/// A 2d sampler that uses a Lanczos filter of configurable size.
// XXX this relies on the user to not call setLut[XYs]... It seems annoying
// to prevent that without having a lot of unnecessary clutter in the code,
// though.
template <class T>
class LanczosSampler : public ConvolutionSampler<T> {
 public:
  typedef typename BaseSampler<T>::Direction Direction;

  /** @brief Constructor with filter size and resolution.
   *
   *  The size of the filter is assumed to be equal to the order.
   *  @param ord  Order of Lanczos filter. The Lanczos filter is given by
   *              sinc(pi*x)*sinc(pi*x/ord), where sinc(x) = sin(x)/x.
   *  @param res  This gives the size of the look-up table for filter values. 
   */
  explicit LanczosSampler(float order = 3, unsigned res = 6000) : order_(order)
  {
    ConvolutionSampler<T>::setSize(order, order);
    ConvolutionSampler<T>::setLuts(detail_lanczos::makeLut(order, res));
  }

  /// Get size of look-up table.
  void getResolution() const { return ConvolutionSampler<T>::getLutX().size(); }

  /// Get order of filter.
  float getOrder() const { return order_; }

 private:
  float                     order_;
};

#endif
