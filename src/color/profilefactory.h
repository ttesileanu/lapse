/** @file profilefactory.h
 *  @brief A factory for CMS profiles.
 */
#ifndef COLOR_PROFILEFACTORY_H_
#define COLOR_PROFILEFACTORY_H_

#include <stdexcept>
#include <string>

#include "profile.h"
#include "transform.h"

/// A profile factory.
class ColorProfileFactory {
 public:
  /// Read profile from file.
  static ColorProfile fromFile(const std::string& fname)
    { return ColorProfile(cmsOpenProfileFromFile(fname.c_str(), "r")); }
  /// Read profile from memory.
  template <class Iterator>
  static ColorProfile fromMemory(Iterator begin, Iterator end) {
    const void* start = static_cast<const void*>(&(*begin));
    size_t len = (end - begin)*sizeof(*begin);

    return ColorProfile(cmsOpenProfileFromMem(start, len));
  }
  /** @brief Make device-link profile.
   *
   *  @param version The profile version to make. Use 3.4 for compatibility with
   *                 older software. Version 4.2 is currently the latest.
   *  @param keepSequence Set to @a true to keep the information about the
   *                      original profiles that have been used in the
   *                      transform. This could take a lot of memory.
   *
   *  See the lcms documentation for @a cmsTransform2DeviceLink for details.
   */
  static ColorProfile fromTransform(const ColorTransform& trafo, float version,
      bool keepSequence = false)
    { return ColorProfile(cmsTransform2DeviceLink(trafo.getHandle(), version,
        keepSequence?cmsFLAGS_KEEP_SEQUENCE:0)); }

  /** @brief Create profile based on built-in data.
   *
   *  Allowed names: sRGB, XYZ, null.
   *  @todo Include all the ones that lCMS allows, and handle options.
   */
  static ColorProfile fromBuiltin(const std::string& name) {
    if (name == "sRGB") {
      return ColorProfile(cmsCreate_sRGBProfile());
/*    } else if (name == "Lab") {
      return ColorProfile(cmsCreateLabProfile());*/
    } else if (name == "XYZ") {
      return ColorProfile(cmsCreateXYZProfile());
    } else if (name == "null") {
      return ColorProfile(cmsCreateNULLProfile());
    } else {
      throw std::runtime_error("Unrecognized built-in profile: " + name);
    }
  }

  /// Save profile to file.
  static void toFile(const ColorProfile& profile, const std::string& name)
    { cmsSaveProfileToFile(profile.getHandle(), name.c_str()); }
  /** @brief Save profile to memory.
   *
   *  Use the @a size method of @a CmsProfile to determine the amount of memory
   *  needed. This function assumes the buffer is large enough to hold all the
   *  data. It returns the number of bytes written.
   */
  template <class Iterator>
  static size_t toMemory(const ColorProfile& profile, Iterator where) {
    cmsUInt32Number size = 0;
    cmsSaveProfileToMem(profile.getHandle(), (void*)(&(*where)), &size);

    return size;
  }

 private:
  // no reason to instantiate this
  ColorProfileFactory() {}
};

#endif
