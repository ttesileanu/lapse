/** @file resizer.h
 *  @brief Image resizer.
 */
#ifndef TRANSFORMS_RESIZER_H_
#define TRANSFORMS_RESIZER_H_

#include <algorithm>
#include <numeric>

#include <boost/shared_ptr.hpp>

#include "image/image.h"
#include "misc/callback.h"
#include "sampler.h"

/// Class that handles resizing of images.
template <class T>
class Resizer {
 public:
  /// A smart pointer to a sampler object.
  typedef boost::shared_ptr<const BaseSampler<T> > SamplerPtr;

  /// Constructor.
  Resizer() : callback_(0), maxThreads_(0) {}
  /// Destructor.
  virtual ~Resizer() {}

  /** @brief Resize the image using the current settings.
   *
   *  Note that, while this always makes an image with image data not shared
   *  with any other image, the metadata is shallow-copied from the original.
   */
  GenericImage<T> resize(const GenericImage<T>& image, unsigned width,
    unsigned height);

  /// Set sampler. This takes ownership of the sampler.
  void setSampler(const BaseSampler<T>* sampler)
    { sampler_ = SamplerPtr(sampler); }
  /// Set sampler.
  void setSampler(const SamplerPtr& sampler) { sampler_ = sampler; }
  /// Get sampler.
  BaseSampler<T>* getSampler() const { return sampler_; }

  /// Set a callback for progress notification.
  void setCallback(Callback* p) { callback_ = p; }

  /** @brief Set maximum number of threads to use.
   *
   *  Use 1 to force single-threaded execution, 0 to use the same number of
   *  threads as the number of hardware threads on the machine.
   */
  void setMaxThreads(size_t n) { maxThreads_ = n; }

 protected:
  /// Call the notification callback, if one was provided.
  bool notifyCallback_(size_t idx, size_t pixels) const {
    if (callback_) {
      pixels_[idx] = pixels;
      size_t allPixels = std::accumulate(pixels_.begin(), pixels_.end(), 0);
      return (*callback_)(float(pixelsOffset_ + allPixels)/totalPixels_);
    } else {
      return true;
    }
  }

  /// The actual resizing function.
  void doResize_(const GenericImage<T>& image, GenericImage<T>& result,
      typename BaseSampler<T>::Direction dir);
  /// The single-threaded resizing function.
  void doResizeST_(const GenericImage<T>& image, GenericImage<T>& result,
      size_t resX1, size_t resY1, size_t resX2, size_t resY2, size_t idx,
      typename BaseSampler<T>::Direction dir);

  SamplerPtr                    sampler_;
  Callback*                     callback_;
  size_t                        pixelsOffset_;
  size_t                        totalPixels_;
  size_t                        maxThreads_;
  mutable std::vector<size_t>   pixels_;
};

#endif
