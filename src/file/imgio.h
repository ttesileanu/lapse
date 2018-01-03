/** @file imgio.h
 *  @brief Generic file loader/writer, identifying files by extension.
 */
#ifndef IMGIO_H_
#define IMGIO_H_

#include <map>
#include <set>

#include <boost/shared_ptr.hpp>

#include "baseio.h"

/** @brief Class to load or write files of various types, identified by filename
 *  extension.
 */
class ImageIO : public BaseIO {
 public:
  /// A smart pointer to a loader.
  typedef boost::shared_ptr<BaseIO> BaseIOPtr;
  /// A set of recognized extensions.
  typedef std::set<std::string> Extensions;

  virtual Image load(const std::string& name) const;
  virtual void write(const std::string& name, const Image& img) const;
  virtual Header inspect(const std::string& name) const;

  /** @brief Register a loader with a given extension.
   *  
   *  The extension should contain the dot.
   */
  void registerType(const std::string& extension, BaseIOPtr loader)
    { loaders_[extension] = loader; }
  /// Register all known types with their default extensions.
  void registerAll();

  /// Get a set of all recognized types.
  Extensions getExtensionList() const;

 private:
  BaseIOPtr getLoader_(const std::string& name) const;

  typedef std::map<std::string, BaseIOPtr> Loaders;

  Loaders       loaders_;
};

#endif
