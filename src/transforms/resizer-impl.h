/** @file resizer-impl.h
 *  @brief Implementation of the image resizer.
 */
#ifndef TRANSFORMS_RESIZER_IMPL_H_
#define TRANSFORMS_RESIZER_IMPL_H_

#include <algorithm>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "resizer.h"

#include "convsampler-impl.h"

template <class T>
GenericImage<T> Resizer<T>::resize(const GenericImage<T>& image, unsigned width,
    unsigned height)
{

  if (width == image.getWidth() && height == image.getHeight()) {
    // nothing to do, return a copy
    GenericImage<T> result(image);
    // only deep-copy the image, not the metadata
    result.makeUnique(GenericImage<T>::SEL_IMAGE);
    return result;
  }

  float scaleX = (float)width / image.getWidth();
  float scaleY = (float)height / image.getHeight();

  GenericImage<T> result;
  result.reshape(width, height);
  result.setChannelCount(image.getChannelCount());
  result.allocate();

  pixelsOffset_ = 0;
  totalPixels_ = result.getWidth()*result.getHeight();

  // if one of the dimensions stays the same, don't change it
  if (width == image.getWidth()) {
    doResize_(image, result, BaseSampler<T>::VERTICAL);
  } else if (height == image.getHeight()) {
    doResize_(image, result, BaseSampler<T>::HORIZONTAL);
  } else {
    // scale one dimension, then the other; the order here is (i think) based
    // on speed; i don't know what the best quality would require
    GenericImage<T> interm;
    interm.setChannelCount(image.getChannelCount());
    if (scaleX < scaleY) {
      interm.reshape(width, image.getHeight());
      interm.allocate();

      size_t partialPixels = interm.getWidth()*interm.getHeight();
      totalPixels_ += partialPixels;

      doResize_(image, interm, BaseSampler<T>::HORIZONTAL);
      pixelsOffset_ = partialPixels;
      doResize_(interm, result, BaseSampler<T>::VERTICAL);
    } else {
      interm.reshape(image.getWidth(), height);
      interm.allocate();

      size_t partialPixels = interm.getWidth()*interm.getHeight();
      totalPixels_ += partialPixels;

      doResize_(image, interm, BaseSampler<T>::VERTICAL);
      pixelsOffset_ = partialPixels;
      doResize_(interm, result, BaseSampler<T>::HORIZONTAL);
    }
  }

  // copy the metadata
  result.copyMetadataFrom(image);

  // copy the type
  result.setChannelTypes(image.getChannelTypes());

  if (callback_)
    (*callback_)(1);

  return result;
}

template <class T>
void Resizer<T>::doResize_(const GenericImage<T>& image,
    GenericImage<T>& result, typename BaseSampler<T>::Direction dir)
{
  if (!sampler_)
    throw std::runtime_error("[Resizer::resize] No sampler set!");

  const unsigned width = result.getWidth();
  const unsigned height = result.getHeight();

  // have as many threads as the hardware allows, but not more than maxThreads_
  // (and treat maxThreads_ == 0 as maxThreads_ == infinity)
  const size_t hwThreads = boost::thread::hardware_concurrency();
  const size_t nThreads0 = (maxThreads_ == 0?hwThreads:
    std::min(hwThreads, maxThreads_));

  // don't have more than one thread for every 4 lines
  const size_t maxDim = std::max(width, height);
  const size_t nThreads = std::max(size_t(1), std::min(nThreads0, maxDim/4));

  pixels_.resize(nThreads);
  std::fill(pixels_.begin(), pixels_.end(), 0);
  if (nThreads == 1) {
    doResizeST_(image, result, 0, 0, width, height, 0, dir);
  } else {
    // need to split the image into nThreads parts; do the split in the longest
    // dimension
    const float step = (float)maxDim / nThreads;

    std::vector<boost::shared_ptr<boost::thread> > threads(nThreads);
    for (size_t i = 0; i < nThreads; ++i) {
      size_t x1, y1, x2, y2;

      if (width > height) {
        x1 = i*step; x2 = (i + 1)*step;
        y1 = 0; y2 = height;
      } else {
        x1 = 0; x2 = width;
        y1 = i*step; y2 = (i + 1)*step;
      }

      threads[i].reset(new boost::thread(&Resizer<T>::doResizeST_, this,
        image, result, x1, y1, x2, y2, i, dir));
    }
    
    // wait for the threads to finish
    for (size_t i = 0; i < nThreads; ++i) {
      threads[i] -> join();
    }
  }
}

template <class T>
void Resizer<T>::doResizeST_(const GenericImage<T>& image,
    GenericImage<T>& result, size_t resX1, size_t resY1,
    size_t resX2, size_t resY2, size_t idx,
    typename BaseSampler<T>::Direction dir)
{
  if (!sampler_)
    throw std::runtime_error("[Resizer::resize] No sampler set!");

  const unsigned width = result.getWidth();
  const unsigned height = result.getHeight();
  const float factorX = (float)image.getWidth() / width;
  const float factorY = (float)image.getHeight() / height;

  const float filterScaleX = ((factorX >= 1)?factorX:1);
  const float filterScaleY = ((factorY >= 1)?factorY:1);

  float origi = resX1*factorX, origj;
  for (unsigned i = resX1; i < resX2; ++i, origi += factorX) {
    origj = resY1*factorY;
    for (unsigned j = resY1; j < resY2; ++j, origj += factorY) {
      sampler_ -> get(image, origi, origj, result(i, j), dir,
          filterScaleX, filterScaleY);
    }
    if (!notifyCallback_(idx, (i - resX1)*height))
      break;
  }
}

#endif
