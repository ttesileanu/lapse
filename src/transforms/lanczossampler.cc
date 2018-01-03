#include "lanczossampler.h"

#include <cmath>

namespace detail_lanczos {

std::vector<float> makeLut(float size, unsigned res)
{
  std::vector<float> result(res);

  const float factor = (float)(2*size) / (res - 1);
  for (unsigned i = 0; i < res; ++i) {
    float x = M_PI*(i*factor - size);

    if (x == 0)
      result[i] = 1;
    else
      result[i] = size * std::sin(x) * std::sin(x/size) / (x*x);
  }

  return result;
}

} // namespace detail_lanczos
