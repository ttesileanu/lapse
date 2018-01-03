#include "linsampler.h"

#include <cmath>

namespace detail_linear {

std::vector<float> makeLut(unsigned res)
{
  std::vector<float> result(res);

  const float factor = 2.0 / (res - 1);
  for (unsigned i = 0; i < res; ++i) {
    float x = std::abs(i*factor - 1);

    result[i] = 1 - x;
  }

  return result;
}

} // namespace detail_linear
