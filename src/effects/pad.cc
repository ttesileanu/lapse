#include "pad.h"

#include <iostream>
#include <stdexcept>

#include "image/image-impl.h"

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

template <class Key, class Container>
typename Container::mapped_type get_item_default(const Container& container,
    const Key& key, typename Container::mapped_type def_value)
{
  typename Container::const_iterator i = container.find(key);
  if (i == container.end())
    return def_value;
  else
    return i -> second;
}

} // anonymous namespace

void PadEffect::operator()(Image8& image,
    const std::map<std::string, double>& props, int verb)
{
  // get the target image size
  size_t im_w = get_item(props, "target_w");
  size_t im_h = get_item(props, "target_h");

  // get the background color, or assume black
  double bkg_r = get_item_default(props, "bkg_r", 0);
  double bkg_g = get_item_default(props, "bkg_g", 0);
  double bkg_b = get_item_default(props, "bkg_b", 0);

  // make an image of the target size and fill it with the background color
  Image8 result;
  result.reshape(im_w, im_h);
  result.setChannelTypes(image.getChannelTypes());
  result.allocate();
  result.copyMetadataFrom(image);

  // XXX from here on assuming channel count is 3 and order is RGB
  if (result.getChannelCount() != 3)
    throw std::runtime_error("Padding assumes RGB images.");

  // position image in center of resulting frame
  // crop if necessary
  int start_x = ((int)im_w - image.getWidth())/2;
  int start_y = ((int)im_h - image.getHeight())/2;
  int end_x = ((int)im_w + image.getWidth())/2;
  int end_y = ((int)im_h + image.getHeight())/2;

  // fill with background color or image, depending on location
  for (size_t i = 0; i < im_w; ++i) {
    for (size_t j = 0; j < im_h; ++j) {
      if (i < start_x || i >= end_x || j < start_y || j >= end_y) {
        // background
        result(i, j)[0] = result.clampColor(bkg_r);
        result(i, j)[1] = result.clampColor(bkg_g);
        result(i, j)[2] = result.clampColor(bkg_b);
      } else {
        // image
        for (size_t k = 0; k < 3; ++k) {
          result(i, j)[k] = image(i - start_x, j - start_y)[k];
        }
      }
    }
  }

  image = result;
}
