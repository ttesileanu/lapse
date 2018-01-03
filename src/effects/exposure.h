/** @file exposure.h
 *  @brief An effect that changes the exposure of a picture.
 */
#ifndef LAPSE_EXPOSURE_H_
#define LAPSE_EXPOSURE_H_

#include <string>
#include <map>

#include "image/image.h"

typedef GenericImage<unsigned char> Image8;

/// Apply an exposure effect.
class ExposureEffect {
 public:
  ExposureEffect() : use_xyz_(true) {}

  /** @brief Set whether to do the exposure change in CIE XYZ color space.
   *
   *  The transformation in CIE XYZ is slower, but it's more accurate. There
   *  are still inaccuracies coming from all the non-linear effects that
   *  digital cameras commonly apply to their JPEGs, but this should be
   *  better than performing the change in sRGB.
   */
  void set_use_xyz(bool b) { use_xyz_ = b; }
  /** @brief Get whether we do the exposure change in CIE XYZ color space.
   *
   *  @see set_use_xyz.
   */
  bool get_use_xyz() const { return use_xyz_; }

  /// Apply exposure effect with the given properties.
  void operator()(Image8&, const std::map<std::string, double>&, int);
  /// Increase exposure by @a ev stops.
  void multiply_exposure(Image8&, double ev, int, bool);

 private:
  bool    use_xyz_;
};

#endif
