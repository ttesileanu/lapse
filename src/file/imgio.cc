#include "imgio.h"

#include <algorithm>
#include <functional>

#include <boost/filesystem.hpp>

#include "jpeg.h"

typedef ImageIO::Image Image;

ImageIO::BaseIOPtr ImageIO::getLoader_(const std::string& name)
    const
{
  std::string extension = boost::filesystem::extension(name);

  Loaders::const_iterator i = loaders_.find(extension);
  if (i == loaders_.end())
    throw std::runtime_error("[ImageIO] Unrecognized extension "
      + extension + ".");
  
  // update loader parameters
  i -> second -> setQuality(writeQuality_);
  i -> second -> setSizeHint(sizeHint_);
  i -> second -> setCallback(callback_);
  i -> second -> setObeyOrientationTag(obeyOrientationTag_);

  return i -> second;
}

Image ImageIO::load(const std::string& name) const
{
  return getLoader_(name) -> load(name);
}

void ImageIO::write(const std::string& name, const Image& img) const
{
  getLoader_(name) -> write(name, img);
}

ImageIO::Header ImageIO::inspect(const std::string& name) const
{
  return getLoader_(name) -> inspect(name);
}

void ImageIO::registerAll()
{
  BaseIOPtr jpegPtr(new JpegIO);

  registerType(".jpg", jpegPtr);
  registerType(".JPG", jpegPtr);
}

template <typename Pair>
struct select1st : public std::unary_function<Pair, typename Pair::first_type> {
  typedef Pair                        argument_type;
  typedef typename Pair::first_type   result_type;

  const typename Pair::first_type& operator()(const Pair& p) const
    { return p.first; }
};

ImageIO::Extensions ImageIO::getExtensionList() const
{
  Extensions result;

  std::transform(loaders_.begin(), loaders_.end(),
    std::inserter(result, result.begin()),
    select1st<Loaders::value_type>());

  return result;
}
