#include "effects/effectfactory.h"

#include "effects/exposure.h"
#include "effects/whitebalance.h"
#include "effects/cropresize.h"
#include "effects/pad.h"

EffectFactory* EffectFactory::instance_ = 0;

EffectFactory::EffectFactory()
{
  add_effect("exposure", ExposureEffect());
  add_effect("whitebalance", WhiteBalanceEffect());
  add_effect("cropresize", CropResizeEffect());
  add_effect("pad", PadEffect());
}

const EffectFactory::Transformation& EffectFactory::get_effect(
  const std::string& name)
{
  auto i = transformations_.find(name);
  if (i == transformations_.end())
    throw std::runtime_error("EffectFactory: effect '" + name + "' not found.");
  return i -> second;
}
