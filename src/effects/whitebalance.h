/** @file whitebalance.h
 *  @brief An effect that changes the white balance.
 */
#ifndef LAPSE_WHITEBALANCE_H_
#define LAPSE_WHITEBALANCE_H_

#include <string>
#include <map>

#include "image/image.h"

typedef GenericImage<unsigned char> Image8;

/// Apply a white balance effect.
class WhiteBalanceEffect {
 public:
  /// Constructor.
  WhiteBalanceEffect() : ref_temp_(5500), overblown_protection_(true),
    use_lms_(true) {}

  /// Set reference color temperature.
  void set_ref_temp(double t) { ref_temp_ = t; }
  /// Get reference color temperature.
  double get_ref_temp() const { return ref_temp_; }

  /** @brief Set whether to do overblown color protection.
   *
   *  Overblown colors have uncertain color. This can pose a problem when
   *  changing white balance: a very bright yellow light, for example, will
   *  look white in a picture, but naively changing white balance will turn
   *  it into a different color. This makes it impossible to obtain a
   *  satisfactory effect when overblown highlights exist in the picture. To
   *  mitigate this problem somwhat, the overblown color protector makes sure
   *  that no color component that is pegged against the upper bound is lowered.
   */
  void set_overblown_protection(bool b) { overblown_protection_ = b; }
  /** @brief Get status of overblown color protection.
   *
   *  @see set_overblown_proection.
   */
  bool get_overblown_protection() const { return overblown_protection_; }

  /** @brief Set whether the transformation is performed in the LMS or XYZ
   *         color space.
   *
   *  The transformation will induce strange color casts if XYZ is not used.
   *  The transformation is somewhat slower in LMS space, though.
   */
  void set_use_lms(bool b) { use_lms_ = b; }
  /** @brief Get whether the transformation uses LMS or XYZ color space.
   *
   *  @see set_use_lms
   */
  bool get_use_lms() const { return use_lms_; }

  /// Apply white balance effect.
  void operator()(Image8&, const std::map<std::string, double>&, int);

 private:
  /// Reference color temperature.
  double    ref_temp_;
  /// Overblown color protection.
  bool      overblown_protection_;
  /// Whether to do the transformation in LMS space.
  bool      use_lms_;
};

#endif
