/** @file exifprops.h
 *  @brief A class to simplify extraction of Exif properties from image.
 */
#ifndef LAPSE_EXIFPROPS_H_
#define LAPSE_EXIFPROPS_H_

// a fix for a warning issues by exiv2
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#include <exiv2/exiv2.hpp>
#pragma clang diagnostic pop

#include "image/image-impl.h"

/// A class to simply extraction of Exif properties from images.
class ExifProperties {
 public:
  /// Constructor.
  template <class T>
  ExifProperties(const GenericImage<T>& image) {
    const Blob& exifBlob = image.getMetadatum("exif").blob;
    Exiv2::ExifParser::decode(data_, (const Exiv2::byte*)(&exifBlob[0]),
      exifBlob.size());
  }

  /// Get the exposure value at ISO 100.
  double get_ev100() const;

  /// Check whether a given Exif key exists.
  bool has_key(const std::string& key) const
    { return data_.findKey(Exiv2::ExifKey(key)) != data_.end(); }

 private:
  Exiv2::ExifData   data_;
};

#endif
