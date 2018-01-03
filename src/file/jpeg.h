/** @file jpeg.h
 *  @brief A jpeg loader/writer based on libjpeg[-turbo].
 */
#ifndef FILE_JPEG_H_
#define FILE_JPEG_H_

// apparently jpeg-turbo needs cstdio to define FILE and size_t before inclusion
#include <cstdio>

#include <jpeglib.h>

#include "baseio.h"

/// This class manages input/output for JPEG files.
class JpegIO : public BaseIO {
 public:
  /// Load a JPEG.
  virtual Image load(const std::string& name) const;
  /// Save a JPEG.
  virtual void write(const std::string& name, const Image& img) const;
  /// Get the header information from a JPEG.
  virtual Header inspect(const std::string& name) const;

  /// Convert @a J_COLOR_SPACE to string description.
  static std::string convertColorspace(J_COLOR_SPACE space);

 private:
  /// Notify callback, if one exists.
  bool notifyCallback_(size_t line, size_t total) const {
    if (callback_) return (*callback_)((float)line / total);
    else return true;
  }
  /// Return the scale_num, scale_denom to use for the current size hint.
  std::pair<size_t, size_t> processSizeHint_(size_t w, size_t h) const;
};

#endif
