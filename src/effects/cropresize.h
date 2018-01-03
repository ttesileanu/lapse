/** @file cropresize.h
 *  @brief An effect to crop and/or resize pictures.
 */
#ifndef LAPSE_CROPRESIZE_H_
#define LAPSE_CROPRESIZE_H_

#include <string>
#include <map>

#include "image/image.h"

typedef GenericImage<unsigned char> Image8;

/// Apply a crop and/or resize effect.
class CropResizeEffect {
 public:
  /// Apply the effect with the properties given.
  void operator()(Image8&, const std::map<std::string, double>&, int);
};

#endif
