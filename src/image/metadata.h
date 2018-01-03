/** @file metadata.h
 *  @brief Metadata storage.
 */
#ifndef IMAGE_METADATA_H_
#define IMAGE_METADATA_H_

#include <map>
#include <vector>

#include <boost/shared_ptr.hpp>

/// A structureless data blob.
typedef std::vector<unsigned char> Blob;
/// One item of metadata.
struct Metadatum {
  /** @brief A string identifier for the metadata.
   *
   *  This can be empty. For some file types, like JPEG, this string identifier
   *  is part of the metadata as stored in the file.
   */
  std::string   id;
  /// The data itself.
  Blob          blob;

  /// Empty constructor.
  Metadatum() {}
  /// Constructor with data, and optional ID.
  explicit Metadatum(const Blob& data, const std::string& s = std::string())
    : id(s), blob(data) {}
};

/** @brief Metadata storage.
 *
 *  This class is designed to do shallow copies when using either the copy
 *  constructor or the assignment operator. Thus, it behaves much like a
 *  reference to the actual metadata. To make a deep copy, use @a clone or
 *  @a makeUnique.
 *
 *  This class is meant to help in the implementation of other classes, and
 *  not to be used on its own.
 */
class Metadata {
 public:
  /// The actual metadata storage.
  typedef std::map<std::string, Metadatum> Contents;

  /// Constructor, making metadata with empty contents.
  Metadata() : contents_(new Contents) {}

  /// Make a deep copy of the metadata.
  Metadata clone() const
    { Metadata result(*this); result.makeUnique(); return result; }
  /// Make sure this object does not share memory with any other.
  void makeUnique() { contents_.reset(new Contents(*contents_)); }
  /// Whether this object shares memory with any other.
  bool isUnique() const { return contents_.unique(); }

  /// Get access to the metadata. (read-only)
  const Contents& getContents() const { return *contents_; }
  /// Get access to the metadata.
  Contents& getContents() { return *contents_; }

 private:
  /// The actual contents.
  boost::shared_ptr<Contents>   contents_;
};

#endif
