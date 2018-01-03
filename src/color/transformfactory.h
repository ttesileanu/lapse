/** @file transformfactory.h
 *  @brief A factory for CMS transforms.
 */
#ifndef COLOR_TRANSFORMFACTORY_H_
#define COLOR_TRANSFORMFACTORY_H_

#include <stdexcept>
#include <string>

#include <stdint.h>

#include "profile.h"
#include "transform.h"

/// Transform a certain data and channel type combination to LCMS type.
template <class T>
int toLcmsType(const std::string& chTypes)
{
  throw std::runtime_error("[toLcmsType] Not defined for this type.");
}

template <>
inline int toLcmsType<unsigned char>(const std::string& chTypes)
{
  if (chTypes == "k")
    return TYPE_GRAY_8;
  else if (chTypes == "rgb")
    return TYPE_RGB_8;
  else if (chTypes == "bgr")
    return TYPE_BGR_8;
  else if (chTypes == "YCC")
    return TYPE_YCbCr_8;
  else if (chTypes == "Lab")
    return TYPE_Lab_8;
  else if (chTypes == "XYZ")
    throw std::runtime_error("[toLcmsType] Unsupported type XYZ for 8-bit "
      "data.");
  else if (chTypes == "cmyk")
    return TYPE_CMYK_8;
  else if (chTypes == "YCCk")
    throw std::runtime_error("[toLcmsType] Unsupported type YCCK for 8-bit "
      "data.");
  else
    throw std::runtime_error("[toLcmsType] Unsupported type.");
}

template <>
inline int toLcmsType<signed char>(const std::string& chTypes)
{
  return toLcmsType<unsigned char>(chTypes);
}

template <>
inline int toLcmsType<int16_t>(const std::string& chTypes)
{
  if (chTypes == "k")
    return TYPE_GRAY_16;
  else if (chTypes == "rgb")
    return TYPE_RGB_16;
  else if (chTypes == "bgr")
    return TYPE_BGR_16;
  else if (chTypes == "YCC")
    return TYPE_YCbCr_16;
  else if (chTypes == "Lab")
    return TYPE_Lab_16;
  else if (chTypes == "XYZ")
    return TYPE_XYZ_16;
  else if (chTypes == "cmyk")
    return TYPE_CMYK_16;
  else if (chTypes == "YCCk")
    throw std::runtime_error("[toLcmsType] Unsupported type YCCK for 16-bit "
      "data.");
  else
    throw std::runtime_error("[toLcmsType] Unsupported type.");
}

template <>
inline int toLcmsType<uint16_t>(const std::string& chTypes)
{
  return toLcmsType<int16_t>(chTypes);
}

template <>
inline int toLcmsType<float>(const std::string& chTypes)
{
  if (chTypes == "k")
    return TYPE_GRAY_FLT;
  else if (chTypes == "rgb")
    return TYPE_RGB_FLT;
  else if (chTypes == "bgr")
    throw std::runtime_error("[toLcmsType] Unsupported type BGR for float "
      "data.");
  else if (chTypes == "YCC")
    throw std::runtime_error("[toLcmsType] Unsupported type YCbCr for float"
      " data.");
  else if (chTypes == "Lab")
    return TYPE_Lab_FLT;
  else if (chTypes == "XYZ")
    return TYPE_XYZ_FLT;
  else if (chTypes == "cmyk")
    return TYPE_CMYK_FLT;
  else if (chTypes == "YCCk")
    throw std::runtime_error("[toLcmsType] Unsupported type YCCK for float "
      "data.");
  else
    throw std::runtime_error("[toLcmsType] Unsupported type.");
}

template <>
inline int toLcmsType<double>(const std::string& chTypes)
{
  if (chTypes == "k")
    return TYPE_GRAY_DBL;
  else if (chTypes == "rgb")
    return TYPE_RGB_DBL;
  else if (chTypes == "bgr")
    throw std::runtime_error("[toLcmsType] Unsupported type BGR for double "
      "data.");
  else if (chTypes == "YCC")
    throw std::runtime_error("[toLcmsType] Unsupported type YCbCr for double"
      " data.");
  else if (chTypes == "Lab")
    return TYPE_Lab_DBL;
  else if (chTypes == "XYZ")
    return TYPE_XYZ_DBL;
  else if (chTypes == "cmyk")
    return TYPE_CMYK_DBL;
  else if (chTypes == "YCCk")
    throw std::runtime_error("[toLcmsType] Unsupported type YCCK for float "
      "data.");
  else
    throw std::runtime_error("[toLcmsType] Unsupported type.");
}

/// A transform factory.
class ColorTransformFactory {
 public:
  /** @brief Create a transform from one profile to another.
   *
   *  The types should use LCMS's @a TYPE_... defines, among which
   *    @a TYPE_RGB_8, @a TYPE_BGR_8, @a TYPE_RGB_DBL, @a TYPE_XYZ_FLT,
   *    @a TYPE_Lab_FLT, etc.
   *
   *  The intent can be one of
   *    @a INTENT_PERCEPTUAL, @a INTENT_RELATIVE_COLORIMETRIC, 
   *    @a INTENT_SATURATION, @a INTENT_ABSOLUTE_COLORIMETRIC
   *  as well as any of the other options allowed by LCMS.
   *
   *  By default, this function optimizes the transform before returning; this
   *  can take additional time which is unnecessary if only transforming a small
   *  number of pixels. Pass @a false to @a optimize to avoid the optimization
   *  and save time in this case. See the LCMS documentation for more details.
   */
  static ColorTransform fromProfiles(const ColorProfile& profile1, int type1,
      const ColorProfile& profile2, int type2, int intent, bool optimize = true)
  {
    return ColorTransform(cmsCreateTransform(
      profile1.getHandle(),
      type1,
      profile2.getHandle(),
      type2,
      intent,
      (optimize?0:cmsFLAGS_NOOPTIMIZE)));
  }
  /** @brief Create a transform from profile to profile using image type data.
   *
   *  The @a image1 and @a image2 parameters need to be types that have a
   *  subtype @a value_type, identifying the kind of elements stored, and
   *  a member function @a getChannelTypes() to retrieve channel types as a
   *  string.
   */
  template <class Image1, class Image2>
  static ColorTransform fromProfiles(const ColorProfile& profile1,
      const Image1& image1,
      const ColorProfile& profile2, const Image2& image2, int intent,
      bool optimize = true)
  {
    return fromProfiles(
      profile1,
      toLcmsType<typename Image1::value_type>(image1.getChannelTypes()),
      profile2,
      toLcmsType<typename Image2::value_type>(image2.getChannelTypes()),
      intent, optimize);
  }

  /** @brief Create a transform from a device-link profile.
   *
   *  The @a type1, @a type2, @a intent, and @a optimize parameters are the same
   *  as for @a fromProfiles.
   *
   *  @see fromProfiles.
   */
  static ColorTransform fromDeviceLink(const ColorProfile& profile, int type1,
      int type2, int intent, bool optimize = true)
  {
    cmsHPROFILE handle = profile.getHandle();

    return ColorTransform(cmsCreateMultiprofileTransform(
      &handle, 1, type1, type2, intent, (optimize?0:cmsFLAGS_NOOPTIMIZE)));
  }
  /** @brief Create a transform from a device-link profile using image type
   *         data.
   *
   *  The @a image1 and @a image2 parameters need to be types that have a
   *  subtype @a value_type, identifying the kind of elements stored, and
   *  a member function @a getChannelTypes() to retrieve channel types as a
   *  string.
   */
  template <class Image1, class Image2>
  static ColorTransform fromDeviceLink(const ColorProfile& profile,
      const Image1& image1,
      const Image2& image2, int intent, bool optimize = true)
  {
    return fromDeviceLink(
      profile,
      toLcmsType<typename Image1::value_type>(image1.getChannelTypes()),
      toLcmsType<typename Image2::value_type>(image2.getChannelTypes()),
      intent, optimize);
  }

  /** @brief Create a transform for device proofing.
   *
   *  This maps @a profile1 to @a profile2 such that the output imitates the
   *  look on a third, @a proofing profile.
   *
   *  The @a type1, @a type2, and @a intent parameters are the same as for
   *  @a fromProfiles. The @a flags can be chosen from @a cmsFLAGS_SOFTPROOFING
   *  and cmsFLAGS_GAMUTCHECK. See the LCMS documentation.
   *  @a todo Provide better support for proofing transforms.
   *
   *  @see fromProfiles.
   */
  static ColorTransform forProofing(const ColorProfile& profile1, int type1,
      const ColorProfile& profile2, int type2, const ColorProfile& proofing,
      int intent, int proofIntent = INTENT_ABSOLUTE_COLORIMETRIC, int flags = 0)
  {
    return ColorTransform(cmsCreateProofingTransform(
      profile1.getHandle(),
      type1,
      profile2.getHandle(),
      type2,
      proofing.getHandle(),
      intent,
      proofIntent,
      flags)
    );
  }
  /** @brief Create a device proofing transform using image type data.
   *
   *  The @a image1 and @a image2 parameters need to be types that have a
   *  subtype @a value_type, identifying the kind of elements stored, and
   *  a member function @a getChanelTypes() to retrieve channel type as a
   *  string.
   */
  template <class Image1, class Image2>
  static ColorTransform forProofing(const ColorProfile& profile1,
      const Image1& image1,
      const ColorProfile& profile2, const Image2& image2,
      const ColorProfile& proofing,
      int intent, int proofIntent = INTENT_ABSOLUTE_COLORIMETRIC, int flags = 0)
  {
    return forProofing(
      profile1,
      toLcmsType<typename Image1::value_type>(image1.getChannelTypes()),
      profile2,
      toLcmsType<typename Image2::value_type>(image2.getChannelTypes()),
      proofing, intent, proofIntent, flags);
  }

 private:
  // no reason to instantiate this
  ColorTransformFactory() {}
};

#endif
