#include "exposure.h"

#include <iostream>
#include <stdexcept>

#include <cmath>

#include "color/profilefactory.h"
#include "color/transformfactory.h"
#include "image/image-impl.h"
#include "exifprops/exifprops.h"

typedef GenericImage<float> Image32;

namespace {

template <class Key, class Container>
typename Container::mapped_type get_item(const Container& container,
    const Key& key)
{
  typename Container::const_iterator i = container.find(key);
  if (i == container.end())
    throw std::runtime_error("Key not found.");
  return i -> second;
}

} // anonymous namespace

void ExposureEffect::operator()(Image8& image,
    const std::map<std::string, double>& props, int verb)
{
  if (props.count("use_xyz") > 0) {
    use_xyz_ = get_item(props, "use_xyz") >= 0.5;
  }
  if (props.count("ev100") > 0) {
    // setting the exposure in absolute units
    // we have to first calculate the exposure for the image
    const ExifProperties exif_props(image);
    const double image_ev100 = exif_props.get_ev100();
    const double target_ev100 = get_item(props, "ev100");
    if (verb >= 2) {
      std::cout << "current EV100=" << image_ev100 << " -> " << target_ev100
                << "   ";
    }
    multiply_exposure(image, image_ev100 - target_ev100, verb, use_xyz_);
  } else if (props.count("evrel") > 0) {
    const double evrel = get_item(props, "evrel");
    if (verb >= 2) {
      std::cout << "exposure   ";
    }
    multiply_exposure(image, evrel, verb, use_xyz_);
  };
}

namespace {

template <class T>
void multiply_image(GenericImage<T>& image, double factor)
{
  typename GenericImage<T>::value_type* data = image.getData();
  const size_t n = image.getSize();
  for (size_t i = 0; i < n; ++i)
    data[i] = GenericImage<T>::clampColor((double)data[i]*factor);
}

} // anonymous namespace

void ExposureEffect::multiply_exposure(Image8& image8, double ev, int verb,
    bool xyz)
{
  const double factor = std::pow(2, ev);

  if (verb >= 2) {
    std::cout << "(" << (ev >= 0?"+":"") << ev << "EV, *" << factor << ")"
              << std::endl;
  }

  if (xyz) {
    ColorProfile sRGB = ColorProfileFactory::fromBuiltin("sRGB");
    ColorProfile XYZ = ColorProfileFactory::fromBuiltin("XYZ");

    // convert the image to XYZ first
    Image32 image32;
    image32.reshape(image8.getWidth(), image8.getHeight());
    image32.setChannelCount(3);
    image32.setChannelTypes("XYZ");
    image32.allocate();

    ColorTransform transform = ColorTransformFactory::fromProfiles(
      sRGB, image8, XYZ, image32, INTENT_PERCEPTUAL);
    transform.apply(image8.getData(), image32.getData(),
      image8.getWidth()*image8.getHeight());

    multiply_image(image32, factor);

    // convert back to sRGB
    ColorTransform transform_back = ColorTransformFactory::fromProfiles(
      XYZ, image32, sRGB, image8, INTENT_PERCEPTUAL);
    transform_back.apply(image32.getData(), image8.getData(),
      image8.getWidth()*image8.getHeight());
  } else {
    multiply_image(image8, factor);
  }
}
