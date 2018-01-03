/** @file processor.h
 *  @brief Defines the class that handles the image processing.
 */
#ifndef LAPSE_PROCESSOR_H_
#define LAPSE_PROCESSOR_H_

#include <vector>
#include <string>
#include <map>

#include <boost/lexical_cast.hpp>

#include "image/image.h"

/// A convenient definition.
typedef std::vector<std::string> strings;

/// A set of keyframes is a map from keyframe index to value.
typedef std::map<int, double> Keyframes;
/// A set of properties is a map from property names to @a Keyframes.
typedef std::map<std::string, Keyframes> Properties;
// Each transformation is mapped to its own @a Properties.
typedef std::map<std::string, Properties> EffectsMap;
/// Keep both maps to keyframes and the order in which transformations appeared.
struct Effects {
  /// The mapping from transformation names, to properties, to keyframes.
  EffectsMap                map;
  /// The order in which the transformations first appeared.
  std::vector<std::string>  order;
};

/// Class handling the processing of images.
class Processor {
 public:
  Processor() : verbosity_(1) {}

  /// Add files to the list.
  void add_files(const strings& more)
    { files_.insert(files_.end(), more.begin(), more.end()); }
  /// Parse an effects string, and store them.
  void parse_effects(const std::string& effects);
  /// Run the processor.
  void run();

  /// Set verbosity level.
  void set_verbosity(int v) { verbosity_ = v; }
  /// Get verbosity level.
  int get_verbosity() const { return verbosity_; }

  /// Set output file name template.
  void set_output(const std::string& s) { output_template_ = s; }
  /// Get output file name template.
  std::string get_output() const { return output_template_; }

 private:
  /// The list of files we're working with.
  strings           files_;
  /// The structure holding the effects to be applied.
  Effects           effects_;
  /// Verbosity level.
  int               verbosity_;
  /// Template for output file names.
  std::string       output_template_;
};

#endif
