#include "whitebalance.h"

#include <iostream>
#include <stdexcept>

#include "color/profilefactory.h"
#include "color/transformfactory.h"
#include "image/image-impl.h"

typedef GenericImage<float> Image32;

namespace {

template <class Key, class Container>
inline typename Container::mapped_type get_item(const Container& container,
    const Key& key)
{
  typename Container::const_iterator i = container.find(key);
  if (i == container.end())
    throw std::runtime_error("Key not found.");
  return i -> second;
}

template <class T>
inline T sqr(T x)
{
  return x*x;
}

template <class T>
inline T cube(T x)
{
  return x*x*x;
}

struct Color {
  double x, y;
};

struct Color3 {
  double X, Y, Z;
};

inline double dot(const Color3& a, const Color3& b)
{
  return a.X*b.X + a.Y*b.Y + a.Z*b.Z;
}

inline Color3 to_lms(const Color3& color)
{
  Color3 res;

  res.X = dot(Color3{0.7328, 0.4296, -0.1624}, color);
  res.Y = dot(Color3{-0.7036, 1.6975, 0.0061}, color);
  res.Z = dot(Color3{0.0030, 0.0136, 0.9834}, color);

  return res;
}

// return the LMS color from (x,y) assuming Y = 1
inline Color3 to_lms(const Color& col2)
{
  return to_lms(Color3{col2.x/col2.y, 1, (1 - col2.x - col2.y)/col2.y});
}

inline Color3 to_xyz(const Color3& color)
{
  Color3 res;

  res.X = dot(Color3{1.0961, -0.2789, 0.1827}, color);
  res.Y = dot(Color3{0.4544, 0.4735, 0.0721}, color);
  res.Z = dot(Color3{-0.0096, -0.0057, 1.0153}, color);

  return res;
}

inline Color get_color_from_temp(double t)
{
  // XXX should find more accurate formulas, maybe
  if (t < 1667 || t > 25000)
    return Color{0, 0};
  const double x = (t < 4000?
    (-0.2661239e9/cube(t) - 0.2343580e6/sqr(t) + 0.8776956e3/t + 0.179910):
    (-3.0258469e9/cube(t) + 2.1070379e6/sqr(t) + 0.2226347e3/t + 0.240390));
  const double y = (t < 2222?
    (-1.1063814*cube(x) - 1.34811020*sqr(x) + 2.18555832*x - 0.20219683):
      (t < 4000?
      (-0.9549476*cube(x) - 1.37418593*sqr(x) + 2.09137015*x - 0.16748867):
      ( 3.0817580*cube(x) - 5.87338670*sqr(x) + 3.75112997*x - 0.37001483)));

  return Color{x, y};
}

inline std::ostream& operator<<(std::ostream& out, const Color& color)
{
  return out << "(" << color.x << "," << color.y << ")";
}

void shift(Image32& image, const Color& old_color, const Color& new_color,
    bool, bool lms)
{
  const Color color_factor{new_color.x/old_color.x, new_color.y/old_color.y};
  Color3 factors;
  if (lms) {
    Color3 old_color3 = to_lms(old_color);
    Color3 new_color3 = to_lms(new_color);

    factors.X = new_color3.X / old_color3.X;
    factors.Y = new_color3.Y / old_color3.Y;
    factors.Z = new_color3.Z / old_color3.Z;
  }

  for (size_t j = 0; j < image.getHeight(); ++j) {
    for (size_t i = 0; i < image.getWidth(); ++i) {
      float* p = image(i, j);
      const float sum = p[0] + p[1] + p[2];
      if (!lms) {
        p[2] = (sum - color_factor.x*p[0] - color_factor.y*p[1])/color_factor.y;
        p[0] *= color_factor.x/color_factor.y;
      } else {
        // XXX should use an adaptation <1!
        // need to convert to LMS, do the transformation there, then go back
        Color3 lms_color = to_lms(Color3{p[0], p[1], p[2]});

        lms_color.X *= factors.X;
        lms_color.Y *= factors.Y;
        lms_color.Z *= factors.Z;

        const Color3 new_xyz = to_xyz(lms_color);
        p[0] = new_xyz.X; p[1] = new_xyz.Y; p[2] = new_xyz.Z;
      }
    }
  }
}

void shift(Image8& image8, const Color& old_color, const Color& new_color,
    bool protect, bool lms)
{
  Image8 mask;
  if (protect) {
    // identify overblown channels
    mask.reshape(image8.getWidth(), image8.getHeight());
    mask.setChannelCount(image8.getChannelCount());
    mask.setChannelTypes(image8.getChannelTypes());
    mask.allocate();

    const size_t n = image8.getSize();
    const Image8::value_type* data0 = image8.getData();
    Image8::value_type* pmask = mask.getData();
    for (size_t i = 0; i < n; ++i) {
      *pmask++ = (*data0++ == 255?1:0);
    }
  }

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

  shift(image32, old_color, new_color, protect, lms);

  // convert back to sRGB
  ColorTransform transform_back = ColorTransformFactory::fromProfiles(
    XYZ, image32, sRGB, image8, INTENT_PERCEPTUAL);
  transform_back.apply(image32.getData(), image8.getData(),
    image8.getWidth()*image8.getHeight());

  if (protect) {
    // make sure overblown channels stay overblown
    const size_t n = image8.getSize();
    Image8::value_type* data0 = image8.getData();
    const Image8::value_type* pmask = mask.getData();
    for (size_t i = 0; i < n; ++i, ++data0)
      if (*pmask++) *data0 = 255;
  }
}

} // anonymous namespace

void WhiteBalanceEffect::operator()(Image8& image,
    const std::map<std::string, double>& props, int verb)
{
  if (props.count("overblow_prot") > 0) {
    overblown_protection_ = get_item(props, "overblow_prot") >= 0.5;
  }
  if (props.count("use_lms") > 0) {
    use_lms_ = get_item(props, "use_lms") >= 0.5;
  }

  if (props.count("xrel") > 0 && props.count("yrel") > 0) {
    const Color color_factor{get_item(props, "xrel"), get_item(props, "yrel")};
    if (verb >= 2) {
      std::cout << "Shifting colors by multiplying (x, y) by " << color_factor
                << std::endl;
    }

    // this wouldn't do the right thing in LMS, so we disable that
    shift(image, Color{1, 1}, color_factor, overblown_protection_, false);
  } else {
    // decide on an origin color
    Color old_color;

    bool use_ref_temp = false;
    bool use_target_temp = false;

    if (props.count("srcr") > 0 && props.count("srcg") > 0 &&
        props.count("srcb") > 0) {
      // grrr, need to transform from RGB!
      const unsigned char old_color_rgb[3] = {
          (unsigned char)get_item(props, "srcr"),
          (unsigned char)get_item(props, "srcg"),
          (unsigned char)get_item(props, "srcb")};
      Color3 old_color3;

      ColorProfile sRGB = ColorProfileFactory::fromBuiltin("sRGB");
      ColorProfile XYZ = ColorProfileFactory::fromBuiltin("XYZ");

      ColorTransform transform = ColorTransformFactory::fromProfiles(
        sRGB, TYPE_RGB_8, XYZ, TYPE_XYZ_DBL, INTENT_PERCEPTUAL);
      transform.apply(old_color_rgb, &old_color3, 1);

      const double csum = old_color3.X + old_color3.Y + old_color3.Z;
      old_color = Color{old_color3.X/csum, old_color3.Y/csum};
    } else {
      if (ref_temp_ < 1667 || ref_temp_ > 25000)
        return;
      old_color = get_color_from_temp(ref_temp_);
      use_ref_temp = true;
    }

    // decide on a target color
    Color new_color;
    double new_temp = 0;

    if (props.count("temp") > 0) {
      new_temp = get_item(props, "temp");
      if (new_temp < 1667 || new_temp > 25000)
        return;
      new_color = get_color_from_temp(new_temp);

      use_target_temp = true;
    } else if (props.count("x") > 0 && props.count("y") > 0) {
      new_color = Color{get_item(props, "x"), get_item(props, "y")};
    } else {
      ColorProfile sRGB = ColorProfileFactory::fromBuiltin("sRGB");
      ColorProfile XYZ = ColorProfileFactory::fromBuiltin("XYZ");

      ColorTransform transform = ColorTransformFactory::fromProfiles(
        sRGB, TYPE_RGB_8, XYZ, TYPE_XYZ_DBL, INTENT_PERCEPTUAL);
      const unsigned char white_color_rgb[3] = {128, 128, 128};
      Color3 white_color3;
      transform.apply(white_color_rgb, &white_color3, 1);

      const double csum_w = white_color3.X + white_color3.Y + white_color3.Z;
      new_color = Color{white_color3.X/csum_w, white_color3.Y/csum_w};
    }

    if (verb >= 2) {
      std::cout << "Shifting colors from " << old_color << " to "
                << new_color;
      if (use_ref_temp && use_target_temp) {
        std::cout << " (shifting color temperature from " << ref_temp_
                  << " to " << new_temp << ")";
      }
      std::cout << std::endl;
    }
    shift(image, old_color, new_color, overblown_protection_, use_lms_);
  }
}
