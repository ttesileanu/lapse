/** @file baseio.h
 *  @brief Base class for all image input/output handlers.
 */
#ifndef FILE_BASEIO_H_
#define FILE_BASEIO_H_

#include <string>
#include <utility>

#include "image/image.h"
#include "misc/callback.h"

/// This is the base class for input/output of 8-bit image files.
class BaseIO {
 public:
  /// Information from an image's file header.
  struct Header {
    size_t        width;
    size_t        height;
    size_t        ncomps;
    std::string   colorspace;
    // XXX need to add some std::map or something that allows extra detail for
    // some file types
  };
  /// Image type written and read by this class (or descendants).
  typedef GenericImage<unsigned char> Image;

  /// Constructor.
  BaseIO() : writeQuality_(95), sizeHint_(0, 0), callback_(0),
    obeyOrientationTag_(true) {}
  // Virtual destructor.
  virtual ~BaseIO() {}

  /// Load an image from file. Should be implemented by descendants.
  virtual Image load(const std::string& name) const = 0;
  /// Write an image to file. Should be implemented by descendants.
  virtual void write(const std::string& name, const Image& img) const = 0;
  /** @brief Get some information about the file without reading it all in
             memory.
   *
   *  Should be implemented by descendants.
   */
  virtual Header inspect(const std::string& name) const = 0;

  /// Set a callback functor to notify of read/write progress.
  void setCallback(Callback* callback) { callback_ = callback; }
  /// Get a pointer to the callback functor.
  Callback* getCallback() const { return callback_; }

  /// Set save quality for file types that support it.
  void setQuality(int q) { writeQuality_ = q; }
  /// Get save quality for file types that support it.
  int getQuality() const { return writeQuality_; }

  /** @brief Set load size hint.
   *
   *  For some file types (e.g., JPEG), loading can be accelerated when the
   *  image we need is at a lower resolution than the one it is saved at. This
   *  can achieve much better speeds (at comparable or better quality) than
   *  loading the full image, and then resizing. Set the size hint to the
   *  approximate intended final size of the image. The load is not guaranteed
   *  to obey this size, but the image returned should be either at the image
   *  resolution, or at a resolution that is larger than the hint.
   */
  void setSizeHint(size_t x, size_t y)
    { sizeHint_.first = x; sizeHint_.second = y; }
  /// Set load size hint.
  void setSizeHint(const std::pair<size_t, size_t>& s) { sizeHint_ = s; }

  /// Set whether to obey orientation tags (e.g., EXIF) when loading.
  void setObeyOrientationTag(bool b) { obeyOrientationTag_ = b; }

 protected:
  int                       writeQuality_;
  std::pair<size_t, size_t> sizeHint_;
  Callback*                 callback_;
  bool                      obeyOrientationTag_;
};

#endif
