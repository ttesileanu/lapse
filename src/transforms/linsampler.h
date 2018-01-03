/** @file linsampler.h
 *  @brief Linear (triangular) filter.
 */
#ifndef TRANSFORMS_LINSAMPLER_H_
#define TRANSFORMS_LINSAMPLER_H_

#include <vector>

#include "sampler.h"
#include "convsampler.h"

namespace detail_linear {

  /// Calculate the look-up table.
  extern std::vector<float> makeLut(unsigned res);

} // namespace detail_linear

/// A 2d sampler that uses a linear (triangular) filter.
// XXX this relies on the user to not call setLut[XYs]... It seems annoying
// to prevent that without having a lot of unnecessary clutter in the code,
// though.
template <class T>
class LinearSampler : public ConvolutionSampler<T> {
 public:
  typedef typename BaseSampler<T>::Direction Direction;

  /// Constructor with resolution.
  explicit LinearSampler(unsigned res = 6000)
  {
    ConvolutionSampler<T>::setSize(1, 1);
    ConvolutionSampler<T>::setLuts(detail_linear::makeLut(res));
  }

  /// Get size of look-up table.
  void getResolution() const { return ConvolutionSampler<T>::getLutX().size(); }
};

#endif
