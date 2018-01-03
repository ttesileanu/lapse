#include "exifprops.h"

#include <stdexcept>

#include <cmath>

#include "image/image-impl.h"

namespace {

template <class T>
T sqr(T x)
{
  return x*x;
}

} // anonymous namespace

double ExifProperties::get_ev100() const
{
/*  auto i = data_.findKey(Exiv2::ExifKey("Exif.Photo.ApertureValue"));
  double Av;
  if (i != data_.end()) {
    Av = i -> toFloat();
  } else {
    i = data_.findKey(Exiv2::ExifKey("Exif.Photo.FNumber"));
    if (i == data_.end())
      throw std::runtime_error("Couldn't find aperture value information.");
    Av = std::log(sqr(i -> toFloat())) / std::log(2);
  }

  i = data_.findKey(Exiv2::ExifKey("Exif.Photo.ShutterSpeedValue"));
  double Tv;
  if (i != data_.end()) {
    Tv = i -> toFloat();
  } else {
    i = data_.findKey(Exiv2::ExifKey("Exif.Photo.ExposureTime"));
    if (i == data_.end())
      throw std::runtime_error("Couldn't find shutter speed information.");
    Tv = -std::log(i -> toFloat()) / std::log(2);
  } */

  auto i = data_.findKey(Exiv2::ExifKey("Exif.Photo.FNumber"));
  if (i == data_.end())
    throw std::runtime_error("Couldn't find aperture value information.");
  const double Av = std::log(sqr(i -> toFloat())) / std::log(2);

  i = data_.findKey(Exiv2::ExifKey("Exif.Photo.ExposureTime"));
  if (i == data_.end())
    throw std::runtime_error("Couldn't find shutter speed information.");
  const double Tv = -std::log(i -> toFloat()) / std::log(2);

  i = data_.findKey(Exiv2::ExifKey("Exif.Photo.ISOSpeedRatings"));
  if (i == data_.end())
    throw std::runtime_error("Couldn't find ISO information.");
  const double Sv = std::log(i -> toFloat() / 100) / std::log(2);

  return Av + Tv - Sv;
}
