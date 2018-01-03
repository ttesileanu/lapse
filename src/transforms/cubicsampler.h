/** @file cubicsampler.h
 *  @brief Cubic filter.
 */
#ifndef TRANSFORMS_CUBICSAMPLER_H_
#define TRANSFORMS_CUBICSAMPLER_H_

#include <vector>

#include "sampler.h"
#include "convsampler.h"

namespace detail_cubic {

  /// Calculate the look-up table.
  extern std::vector<float> makeLut(float B, float C, unsigned res);

} // namespace detail_cubic

/// A 2d sampler that uses a bicubic filter.
// XXX this relies on the user to not call setLut[XYs]... It seems annoying
// to prevent that without having a lot of unnecessary clutter in the code,
// though.
template <class T>
class CubicSampler : public ConvolutionSampler<T> {
 public:
  typedef typename BaseSampler<T>::Direction Direction;

  /** @brief Constructor with filter parameters and resolution.
   *
   *  The default sizes are chosen from the recommended filter of
   *  Mitchell and Netravali paper.
   */
  explicit CubicSampler(float B = 1.0/3, float C = 1.0/3,
      unsigned res = 6000) : B_(B), C_(C)
  {
    ConvolutionSampler<T>::setSize(2, 2);
    ConvolutionSampler<T>::setLuts(detail_cubic::makeLut(B, C, res));
  }

  /// Get size of look-up table.
  void getResolution() const { return ConvolutionSampler<T>::getLutX().size(); }

  /// Get B parameter of filter.
  float getB() const { return B_; }
  /// Get C parameter of filter.
  float getC() const { return C_; }

 private:
  float                     B_, C_;
};

#endif
