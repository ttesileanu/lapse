/** @file imgbuffer-impl.h
 *  @brief Implementation of some functions in imgbuffer.h.
 */
#ifndef IMAGE_IMGBUFFER_IMPL_H_
#define IMAGE_IMGBUFFER_IMPL_H_

#include "imgbuffer.h"

#include <algorithm>

template <class T>
void ImageBuffer<T>::forceCopy_()
{
  size_t size = getSize();
  if (size == 0) {
    clear();
    return;
  }

  // make a new data buffer
  T* newbuffer = new T[size];

  // new strides
  int ns1 = ncomps_;
  int ns2 = ncomps_*width_;

  // copy the data
  for (size_t i = 0; i < height_; ++i) {
    for (size_t j = 0; j < width_; ++j) {
      const T* first = ptr_ + j*strides_[0] + i*strides_[1];
      std::copy(first, first + ncomps_, newbuffer + j*ns1 + i*ns2);
    }
  }

  // update the data pointer and the strides
  ptr_ = newbuffer;
  strides_[0] = ns1; strides_[1] = ns2;
  data_.reset(newbuffer);
}

template <class T>
void ImageBuffer<T>::allocate()
{
  size_t size = getSize();
  if (size == 0)
    clear();
  else {
    data_.reset(new T[size]);
    ptr_ = data_.get();
    strides_[0] = ncomps_;
    strides_[1] = ncomps_*width_;
  }
}

template <class T>
void ImageBuffer<T>::crop(size_t offsetX, size_t offsetY, size_t width,
    size_t height)
{
  ptr_ += offsetX*strides_[0] + offsetY*strides_[1];

  width_ = ((width != 0) ? width : (width_ - offsetX));
  height_ = ((height != 0) ? height : (height_ - offsetY));
}

template <class T>
void ImageBuffer<T>::coarseRotate(int nRot)
{
  if (isEmpty())
    return;
  if (width_ == 0 || height_ == 0) // nothing to do
    return;

  // this only makes sense modulo 4
  nRot = nRot % 4;
  // C modulo doesn't do the right thing for negative numbers
  if (nRot < 0)
    nRot += 4;

  if (nRot == 0) // nothing to do
    return;

  int tmp;
  switch (nRot) {
    case 1:
      // i want (x, y) -> (y, W' - x - 1) in the original coordinates
      // p' + x*s0' + y*s1' = p+y*s0+(W'-x-1)*s1 = p + (W'-1)*s1 - x*s1 + y*s0
      std::swap(width_, height_);
      ptr_ += (width_ - 1)*strides_[1];
      tmp = strides_[0];
      strides_[0] = -strides_[1];
      strides_[1] = tmp;
      break;
    case 2:
      // i want (x, y) -> (W' - x - 1, H' - y - 1) in the original coordinates
      // p' + x*s0' + y*s1' = p + (W'-x-1)*s0 + (H'-y-1)*s1
      //   = p + (W'-1)*s0 + (H'-1)*s1 - x*s0 - y*s1
      ptr_ += (width_ - 1)*strides_[0] + (height_ - 1)*strides_[1];
      strides_[0] = -strides_[0];
      strides_[1] = -strides_[1];
      break;
    case 3:
      // i want (x, y) -> (H' - y - 1, x) in the original coordinates
      // p' + x*s0' + y*s1' = p+(H'-y-1)*s0+x*s1 = p + (H'-1)*s0 + x*s1 - y*s0
      std::swap(width_, height_);
      ptr_ += (height_ - 1)*strides_[0];
      tmp = strides_[0];
      strides_[0] = strides_[1];
      strides_[1] = -tmp;
      break;
    default:
      throw std::runtime_error("nRot not 1, 2, 3... this should never happen.");
      // nRot can now only be 1, 2, or 3
  };
}

template <class T>
void ImageBuffer<T>::flip(ImageAxis axis)
{
  if (axis & X_AXIS) {
    // i want (x, y) -> (W - x - 1, y) in the original coordinates
    // p' + x*s0' + y*s1' = p + (W-x-1)*s0 +y*s1 = p + (W-1)*s0 - x*s0 + y*s1
    ptr_ += (width_ - 1)*strides_[0];
    strides_[0] = -strides_[0];
  }
  if (axis & Y_AXIS) {
    // i want (x, y) -> (x, H - y - 1) in the original coordinates
    // p' + x*s0' + y*s1' = p + x*s0 + (H-y-1)*s1 = p + (H-1)*s1 + x*s0 - y*s1
    ptr_ += (height_ - 1)*strides_[1];
    strides_[1] = -strides_[1];
  }
}

template <class T>
void ImageBuffer<T>::flipXY()
{
  std::swap(strides_[0], strides_[1]);
}

#endif
