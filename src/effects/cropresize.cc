#include "cropresize.h"

#include <iostream>
#include <stdexcept>

#include "transforms/resizer.h"
#include "transforms/lanczossampler.h"
#include "transforms/cubicsampler.h"

#include "image/image-impl.h"
#include "transforms/convsampler-impl.h"
#include "transforms/resizer-impl.h"

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

struct Point {
  size_t x, y;
};

struct Rectangle {
  Point p1, p2;
};

std::ostream& operator<<(std::ostream& out, const Point& point)
{
  return out << "(" << point.x << "," << point.y << ")";
}

std::ostream& operator<<(std::ostream& out, const Rectangle& rect)
{
  return out << rect.p1 << "-" << rect.p2;
}

} // anonymous namespace

void CropResizeEffect::operator()(Image8& image,
    const std::map<std::string, double>& props, int verb)
{
  // default crop region: whole image
  Rectangle crop_region{{0, 0}, {image.getWidth(), image.getHeight()}};
  // adding +0.5 for rounding to nearest integer
  if (props.count("x0") > 0)
    crop_region.p1.x = get_item(props, "x0") + 0.5;
  if (props.count("y0") > 0)
    crop_region.p1.y = get_item(props, "y0") + 0.5;
  if (props.count("x1") > 0)
    crop_region.p2.x = get_item(props, "x1") + 0.5;
  if (props.count("y1") > 0)
    crop_region.p2.y = get_item(props, "y1") + 0.5;
  // the +0.5 added to crop_region.p1 makes sure that crop_region.p2 are also
  // rounded to the nearest integer
  if (props.count("cwidth") > 0)
    crop_region.p2.x = crop_region.p1.x + get_item(props, "cwidth");
  if (props.count("cheight") > 0)
    crop_region.p2.y = crop_region.p1.y + get_item(props, "cheight");
  
  if (crop_region.p1.x != 0 || crop_region.p1.y != 0 ||
      crop_region.p2.x!=image.getWidth() || crop_region.p2.y!=image.getHeight())
  {
    // need to crop
    if (verb >= 2) {
      std::cout << "Cropping to " << crop_region << std::endl;
    }
    image.crop(crop_region.p1.x, crop_region.p1.y,
      crop_region.p2.x - crop_region.p1.x,
      crop_region.p2.y - crop_region.p1.y);
  }

  // default target size: same as crop region
  Point final_size
    {crop_region.p2.x - crop_region.p1.x, crop_region.p2.y - crop_region.p1.y};
  if (props.count("twidth") > 0)
    final_size.x = get_item(props, "twidth") + 0.5;
  if (props.count("theight") > 0)
    final_size.y = get_item(props, "theight") + 0.5;

  if (final_size.x != image.getWidth() || final_size.y != image.getHeight()) {
    // need to resize
    if (verb >= 2) {
      std::cout << "Resizing to " << final_size << std::endl;
    }
    Resizer<Image8::value_type> resizer;
    double factorX = (double)final_size.x / image.getWidth();
    double factorY = (double)final_size.y / image.getHeight();

    if (factorX*factorY < 1)
      resizer.setSampler(new LanczosSampler<Image8::value_type>);
    else
      resizer.setSampler(new CubicSampler<Image8::value_type>);
    image = resizer.resize(image, final_size.x, final_size.y);
  }
}
