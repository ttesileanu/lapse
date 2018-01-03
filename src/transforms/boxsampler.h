/** @file boxsampler.h
 *  @brief Box filter.
 */
#ifndef TRANSFORMS_BOXSAMPLER_H_
#define TRANSFORMS_BOXSAMPLER_H_

#include <vector>

#include "sampler.h"
#include "convsampler.h"

/// A 2d sampler that uses a box filter.
// XXX this relies on the user to not call setLut[XYs]... It seems annoying
// to prevent that without having a lot of unnecessary clutter in the code,
// though.
template <class T>
class BoxSampler : public ConvolutionSampler<T> {
 public:
  typedef typename BaseSampler<T>::Direction Direction;

  /// Constructor.
  BoxSampler()
  {
    ConvolutionSampler<T>::setSize(0.5, 0.5);
    ConvolutionSampler<T>::setLuts(std::vector<float>(1, 1.0));
  }
};

#endif
