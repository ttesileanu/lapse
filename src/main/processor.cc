#include "processor.h"

#include <iostream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "color/profilefactory.h"
#include "color/transformfactory.h"
#include "effects/effectfactory.h"
#include "file/jpeg.h"
#include "image/image-impl.h"

typedef JpegIO::Image Image8;

namespace fs = boost::filesystem;

namespace {

// get part before dot in a trafo.prop expression
std::string get_trafo_part(const std::string& s)
{
  const size_t dpos = s.find('.');
  if (dpos == std::string::npos) return std::string();
  return s.substr(0, dpos);
}

// get part after dot in a trafo.prop expression
std::string get_prop_part(const std::string& s)
{
  const size_t dpos = s.find('.');
  if (dpos == std::string::npos) return s;
  return s.substr(dpos + 1);
}

// simple implementation for C++11's prev
template <class It>
It prev_it(It i)
{
  --i;
  return i;
}

} // anonymous namespace

void Processor::parse_effects(const std::string& effects)
{
  // in general, white space is ignored between tokens
  // we tokenize & parse at the same time
  std::string token;
  const size_t n = effects.size();

  enum State {STARTING, HAD_LHS, HAD_EQUAL};
  State state = STARTING;
  std::string lhs;
  int keyframe = 0;
  for (size_t i = 0; i <= n; ++i) {
    // to avoid extra code, add a virtual space at the end of the string
    const char c = (i < n?effects[i]:' ');
    if (isspace(c) || c == ':' || c == '=' || token == ":" || token == "=") {
      if (!token.empty()) {
        // we got a new token!
        switch (state) {
          case STARTING:
            lhs = token;
            state = HAD_LHS;
            break;
          case HAD_LHS:
            if (token != ":" && token != "=")
              throw std::runtime_error("Parse error at position " + 
                boost::lexical_cast<std::string>(i) + " in effects list: " +
                "expected : or =.");
            if (token == ":") {
              // had a keyframe label
              keyframe = boost::lexical_cast<int>(lhs);
              state = STARTING;
            } else {
              // had first half of assignment
              state = HAD_EQUAL;
            }
            break;
          case HAD_EQUAL:
            const std::string trafo_part = get_trafo_part(lhs);
            effects_.map[trafo_part][get_prop_part(lhs)][keyframe] =
              boost::lexical_cast<double>(token);
            if (find(effects_.order.begin(), effects_.order.end(), trafo_part)
                  == effects_.order.end())
              effects_.order.push_back(trafo_part);
            state = STARTING;
            break;
        }
      }
      if (!isspace(c))
        token = c;
      else
        token.clear();
    } else {
      token += c;
    }
  }
}

void Processor::run()
{
  JpegIO io;
  io.setObeyOrientationTag(false);
  io.setQuality(95);

  // we need an sRGB profile
  ColorProfile sRGB = ColorProfileFactory::fromBuiltin("sRGB");

  fs::path out_path(output_template_);
  std::string out_stem = out_path.stem().native();
  std::string out_ext = out_path.extension().native();
  fs::path out_parent = out_path.parent_path();
  // std::string::npos is guaranteed to be size_type(-1), so adding 1 -> 0
  const size_t xstart = out_stem.find_last_not_of("X") + 1;
  if (xstart >= out_stem.length())
    throw std::runtime_error("Output file name specification invalid.");
  const size_t xlen = out_stem.length() - xstart;

  boost::format formatter("%|0" + boost::lexical_cast<std::string>(xlen) +
    "|");

  const size_t nframes = files_.size();
  for (size_t i = 0; i < nframes; ++i) {
    if (verbosity_ > 0) {
      std::cout << "Working on frame " << i << " (" << files_[i] << ")..."
                << std::endl;
    }

    // load image and transform to sRGB
    Image8 image8 = io.load(files_[i]);

    if (image8.hasMetadatum("icc")) {
      const Blob& icc = image8.getMetadatum("icc").blob;

      // get the profile of the image
      ColorProfile profile = ColorProfileFactory::fromMemory(icc.begin(),
        icc.end());
      ColorTransform transform = ColorTransformFactory::fromProfiles(
          profile, image8, sRGB, image8, INTENT_PERCEPTUAL);

      // apply the transform to the image
      transform.apply(image8.getData(), image8.getData(),
        image8.getWidth()*image8.getHeight());
    }

    // find all the effects for this frame, and apply them
    for (std::string effect_name: effects_.order) {
      auto effect = effects_.map[effect_name];
      PropertyMap properties;
      for (auto prop: effect) {
        const auto k2 = prop.second.upper_bound(i);
        if (k2 != prop.second.begin()) {
          // by definition of 'upper_bound', k1's frame is <= i
          const auto k1 = prev_it(k2);
          if (k2 == prop.second.end()) {
            // we have only one keyframe, so no interpolation
            properties[prop.first] = k1 -> second;
          } else {
            // interpolate
            const double a = double(i - k1->first) / (k2->first - k1->first);
            properties[prop.first] = (1-a)*k1->second + a*k2->second;
          }
        } // otherwise we have only one keyframe which we haven't reached yet
      }

      EffectFactory::get_instance() ->
        get_effect(effect_name)(image8, properties, verbosity_);
    }

    // figure out the name of the output file
    std::ostringstream num_str_stream;
    num_str_stream << formatter % i;
    const std::string num_str = out_stem.substr(0, xstart)+num_str_stream.str();
    std::string out_name = (out_parent / num_str).replace_extension(out_ext).
      native();

    std::cout << "Writing to " << out_name << "..." << std::endl;

    // XXX how do we decide on quality? Can we read it from original file?
    io.write(out_name, image8);
  }
}
