#include "cubicsampler.h"

#include <cmath>

namespace detail_cubic {

std::vector<float> makeLut(float B, float C, unsigned res)
{
  std::vector<float> result(res);

  // filter is P_3 |x|^3 + P_2 |x|^2 + P_0 for |x| < 1
  //           Q_3 |x|^3 + Q_2 |x|^2 + Q_1 |x| + Q_0 for 1 <= x < 2
  // with

  float P3 =  12 - 9*B - 6*C;
  float P2 = -18 + 12*B + 6*C;
  float P0 = 6 - 2*B;
  float Q3 = -B - 6*C;
  float Q2 = 6*B + 30*C;
  float Q1 = -12*B - 48*C;
  float Q0 = 8*B + 24*C;

  const float factor = 4.0 / (res - 1);
  for (unsigned i = 0; i < res; ++i) {
    float x = std::abs(i*factor - 2);
    float x2 = x*x;
    float x3 = x2*x;

    if (x < 1)
      result[i] = P3*x3 + P2*x2 + P0;
    else
      result[i] = Q3*x3 + Q2*x2 + Q1*x + Q0;
  }

  return result;
}

} // namespace detail_cubic
