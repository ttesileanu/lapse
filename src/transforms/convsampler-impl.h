/** @file convsampler-impl.h
 *  @brief Implementation of convolution sampler.
 */
#ifndef TRANSFORMS_CONVSAMPLER_IMPL_H_
#define TRANSFORMS_CONVSAMPLER_IMPL_H_

#include "convsampler.h"

#include "image/image-impl.h"

template <class T>
void ConvolutionSampler<T>::getProduct_(const GenericImage<T>& image, float x,
    float y, T* where, float scalex, float scaley) const
{
  const unsigned ncomps = image.getChannelCount();

  float sizeX = sizeX_*scalex;
  float sizeY = sizeY_*scaley;

  if (sizeX < 0.5)
    sizeX = 0.5;
  if (sizeY < 0.5)
    sizeY = 0.5;

  // find out which pixels we're looking at, and how to map the positions of
  // their centers with respect to x and y to the coordinates in the filter
  // matrix
  // XXX this doesn't properly antialias values that are almost out of bounds
  // XXX the only efficient solution for that is probably a branch in the code,
  // since we don't want to be checking for out-of-bounds for every pixel
  const int startX = std::max(0, (int)std::floor(x - sizeX + 1));
  const int startY = std::max(0, (int)std::floor(y - sizeY + 1));
  const int endX = std::min((int)image.getWidth() - 1,
    (int)std::floor(x + sizeX));
  const int endY = std::min((int)image.getHeight() - 1,
    (int)std::floor(y + sizeY));

  // lengths of the look-up tables
  const unsigned flenX = lutX_.size();
  const unsigned flenY = lutY_.size();

  // scale factors to map from image-space distances to positions in look-up
  // tables
  const float mapfactorX = flenX / (2*sizeX);
  const float mapfactorY = flenY / (2*sizeY);

  // cache shifted x and y values
  const float shiftedX = x + sizeX;
  const float shiftedY = y + sizeY;

  for (unsigned comp = 0; comp < ncomps; ++comp) {
    // calculate the contributions from all the relevant neighbors
    float value = 0;
    // keep track of the sum of weights; this is to make sure that a uniform
    // image always gets sampled as a uniform image, avoiding any sampling
    // errors
    float wsum = 0;
    float mapi = (shiftedX - startX)*mapfactorX, mapj;
    for (int i = startX; i <= endX; ++i, mapi -= mapfactorX) {
//      const float shiftedDistX = shiftedX - i;
//      const unsigned mapi = shiftedDistX*mapfactorX;
      mapj = (shiftedY - startY)*mapfactorY;
      for (int j = startY; j <= endY; ++j, mapj -= mapfactorY) {
//        const float shiftedDistY = shiftedY - j;
//        const unsigned mapj = shiftedDistY*mapfactorY;

        const float weight = lutX_[(int)mapi]*lutY_[(int)mapj];
        value += image(i, j)[comp]*weight;
        wsum += weight;
      }
    }
    
    value /= wsum;
    // clamp to allowed values
    // this is a static member, but it's shorter to write the call this way
    where[comp] = image.clampColor(value);
  }
}

template <class T>
void ConvolutionSampler<T>::getX_(const GenericImage<T>& image, float x,
    float y, T* where, float scalex) const
{
  const unsigned ncomps = image.getChannelCount();

  float sizeX = sizeX_*scalex;

  if (sizeX < 0.5)
    sizeX = 0.5;

  // find out which pixels we're looking at, and how to map the positions of
  // their centers with respect to x to the coordinates in the filter
  // matrix
  // XXX this doesn't properly antialias values that are almost out of bounds
  // XXX the only efficient solution for that is probably a branch in the code,
  // since we don't want to be checking for out-of-bounds for every pixel
  const int startX = std::max(0, (int)std::floor(x - sizeX + 1));
  const int endX = std::min((int)image.getWidth() - 1,
    (int)std::floor(x + sizeX));
  const int valY = y;

  // length of the horizontal look-up table
  const unsigned flenX = lutX_.size();

  // scale factor to map from image-space distances to position in look-up
  // table
  const float mapfactorX = flenX / (2*sizeX);

  // cache shifted x value
  const float shiftedX = x + sizeX;

  for (unsigned comp = 0; comp < ncomps; ++comp) {
    // calculate the contributions from all the relevant neighbors
    float value = 0;
    // keep track of the sum of weights; this is to make sure that a uniform
    // image always gets sampled as a uniform image, avoiding any sampling
    // errors
    float wsum = 0;
    float mapi = (shiftedX - startX)*mapfactorX;
    for (int i = startX; i <= endX; ++i, mapi -= mapfactorX) {
//      const float shiftedDistX = shiftedX - i;
//      const unsigned mapi = shiftedDistX*mapfactorX;

      const float weight = lutX_[(int)mapi];
      value += image(i, valY)[comp]*weight;
      wsum += weight;
    }
    
    value /= wsum;
    // clamp to allowed values
    // this is a static member, but it's shorter to write the call this way
    where[comp] = image.clampColor(value);
  }
}

template <class T>
void ConvolutionSampler<T>::getY_(const GenericImage<T>& image, float x,
    float y, T* where, float scaley) const
{
  const unsigned ncomps = image.getChannelCount();

  float sizeY = sizeY_*scaley;

  if (sizeY < 0.5)
    sizeY = 0.5;

  // find out which pixels we're looking at, and how to map the positions of
  // their centers with respect to x to the coordinates in the filter
  // matrix
  // XXX this doesn't properly antialias values that are almost out of bounds
  // XXX the only efficient solution for that is probably a branch in the code,
  // since we don't want to be checking for out-of-bounds for every pixel
  const int startY = std::max(0, (int)std::floor(y - sizeY + 1));
  const int endY = std::min((int)image.getHeight() - 1,
    (int)std::floor(y + sizeY));
  const int valX = x;

  // length of the horizontal look-up table
  const unsigned flenY = lutY_.size();

  // scale factor to map from image-space distances to position in look-up
  // table
  const float mapfactorY = flenY / (2*sizeY);

  // cache shifted x value
  const float shiftedY = y + sizeY;

  for (unsigned comp = 0; comp < ncomps; ++comp) {
    // calculate the contributions from all the relevant neighbors
    float value = 0;
    // keep track of the sum of weights; this is to make sure that a uniform
    // image always gets sampled as a uniform image, avoiding any sampling
    // errors
    float wsum = 0;
    float mapj = (shiftedY - startY)*mapfactorY;
    for (int j = startY; j <= endY; ++j, mapj -= mapfactorY) {
//      const float shiftedDistY = shiftedY - j;
//      const unsigned mapj = shiftedDistY*mapfactorY;

      const float weight = lutY_[(int)mapj];
      value += image(valX, j)[comp]*weight;
      wsum += weight;
    }
    
    value /= wsum;
    // clamp to allowed values
    // this is a static member, but it's shorter to write the call this way
    where[comp] = image.clampColor(value);
  }
}

#endif
