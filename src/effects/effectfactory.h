/** @file effectfactory.h
 *  @brief Defines a class that can return effect functions based on a string
 *         name.
 */
#ifndef LAPSE_EFFECTFACTORY_H_
#define LAPSE_EFFECTFACTORY_H_

#include <string>
#include <map>
#include <functional>

#include "image/image.h"

/// Map from property names to numbers.
typedef std::map<std::string, double> PropertyMap;

typedef GenericImage<unsigned char> Image8;

/// A singleton class keeping track of all the effects.
class EffectFactory {
 public:
  /// A transformation function.
  typedef std::function<void(Image8&, const PropertyMap&, int)>
    Transformation;
  /// Transformations with their names.
  typedef std::map<std::string, Transformation> Transformations;

  /// Get the singleton instance.
  static EffectFactory* get_instance() {
    if (!instance_) instance_ = new EffectFactory;
    return instance_;
  }

  /// Add a transformation.
  void add_effect(const std::string& name, const Transformation& trafo)
    { transformations_[name] = trafo; }

  /// Get a transformation.
  const Transformation& get_effect(const std::string& name);

 private:
  EffectFactory();

  static EffectFactory*     instance_;
  Transformations           transformations_;
};

#endif
