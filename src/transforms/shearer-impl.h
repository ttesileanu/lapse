/** @file shearer-impl.h
 *  @brief Implementation of the image shearer.
 */
#ifndef TRANSFORMS_SHEARER_IMPL_H_
#define TRANSFORMS_SHEARER_IMPL_H_

#include <algorithm>

#include "shearer.h"

template <class T>
void Resizer<T>::apply(
    const GenericImage<T>& image, GenericImage<T>& result,
    float factor)
{
  return apply(image, factor, result.getWidth()/2.0, result.getHeight()/2.0);
}

template <class T>
std::vector<T> getLine(const GenericImage<T>& image,
    unsigned orig_x1_i, unsigned orig_x1_f, unsigned orig_x2, bool horizontal)
{
  size_t n = orig_x1_f - orig_x1_i;
  std::vector<T> result(n, 0);

  if (orig_x1_f > image.getWidth())
    orig_x1_f = image.getWidth();
  auto i = result.begin();
  if (horionztal) {
    for (unsigned x1 = orig_x1_i; x1 < orig_x1_f; ++x1, ++i) {
      *i = image(x1, orig_x2);
    }
  } else {
    for (unsigned x1 = orig_x1_i; x1 < orig_x1_f; ++x1, ++i) {
      *i = image(orig_x2, x1);
    }
  }

  return result;
}

template <class T>
std::vector<T> shift(const GenericImage<T>& image,
    float orig_x1_i, float orig_x1_f, float orig_x2, bool horizontal)
{
  unsigned orig_x1_i_int = std::floor(orig_x1_i);
  unsigned orig_x1_f_int = std::floor(orig_x1_f);
  auto result = getLine(image, orig_x1_i_int, orig_x1_f_int,
      std::floor(orig_x2), horizontal);

  float shift = orig_x1_i - orig_x1_i_int;
  if (std::abs(shift) > 1e-3) {
    auto result2 = getLine(image, orig_x1_i_int+1, orig_x1_f_int+1,
        std::floor(orig_x2), horizontal);
    auto i2 = result2.begin();
    for (auto i = result.begin(); i != result.end(); ++i, ++i2) {
      *i = image.clampColor(shift*(*i2) + (1-shift)*(*i));
    }
  }

  // XXX should handle fractional orig_x2

  return result;
}
        
template <class T>
void Resizer<T>::apply(
    const GenericImage<T>& image, GenericImage<T>& result,
    float factor, float dest_ctr_x, float dest_ctr_y)
{
  unsigned dest_sz1 = (direction_ == HORIZONTAL)?
      result.getWidth():result.getHeight();
  unsigned dest_sz2 = (direction_ == HORIZONTAL)?
      result.getHeight():result.getWidth();

  unsigned orig_sz1 = (direction_ == HORIZONTAL)?
      image.getWidth():image.getHeight();
  unsigned orig_sz2 = (direction_ == HORIZONTAL)?
      image.getHeight():image.getWidth();

  float orig_c1 = orig_sz1/2.0;
  float orig_c2 = orig_sz2/2.0;

  float dest_c1 = (direction_ == HORIZONTAL)?dest_ctr_x:dest_ctr_y;
  float dest_c2 = (direction_ == HORIZONTAL)?dest_ctr_y:dest_ctr_x;

  size_t ncomps = image.getChannelCount();
  if (result.getChannelCount() != ncomps) {
    throw std::runtime_error("[Shearer::apply] Channel count mismatch between "
      "origin and destination image.");
  }

  // iterate through destination image
  for (unsigned x2 = 0; x2 < dest_sz2; ++x2) {
    float orig_d_x2 = x2 - dest_c2;
    float orig_x2 = orig_c2 + orig_d_x2;
    
    unsigned dest_x1_i = 0;
    unsigned dest_x1_f = dest_w;

    float orig_x1_i = orig_c1 + dest_x1_i - dest_c1 + factor*orig_d_x2;
    float orig_x1_f = orig_c1 + dest_x1_f - dest_c1 + factor*orig_d_x2;

    if (orig_x1_i < 0) {
      orig_x1_i = 0;
      dest_x1_i = std::floor(dest_c1 - orig_c1 + factor*orig_d_x2);
      if (dest_x1_i >= dest_sz1) // line falls completely out of dest image
        continue;
    }
    if (orig_x1_f >= orig_sz1) {
      orig_x1_f = orig_sz1;
      dest_x1_f = std::floor(dest_c1 + orig_sz1 - orig_c1 + factor*orig_d_x2);
      if (dest_x1_f < 0) // line falls completely out of dest image
        continue;
    }

    const auto line = shift(image, orig_x1_i, orig_x1_f, orig_x2,
        direction_ == HORIZONTAL);

    unsigned x1 = dest_x1_i;
    if (direction_ == HORIZONTAL) {
      for (auto i = line.begin(); i != line.end(); ++x1, ++i) {
        std::copy(i, i + ncomps, result(x1, x2));
      }
    } else {
      for (auto i = line.begin(); i != line.end(); ++x1, ++i) {
        std::copy(i, i + ncomps, result(x2, x1));
      }
    }
  }

  // XXX should handle fractional shifts in y
}

#endif
