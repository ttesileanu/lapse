/** @file pad.h
    @brief An effect to pad an image to a given size.
 */
#ifndef LAPSE_PAD_H_
#define LAPSE_PAD_H_

#include "image/image.h"

typedef GenericImage<unsigned char> Image8;

/// Apply padding to an image.
class PadEffect {
 public:
  /// Apply the effect with the properties given.
  void operator()(Image8&, const std::map<std::string, double>&, int);
};

#endif
