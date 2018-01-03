/** @file image.h
 *  @brief A generic image class.
 */
#ifndef IMAGE_IMAGE_H_
#define IMAGE_IMAGE_H_

#include <string>

#include "imgbuffer.h"
#include "metadata.h"

/** @brief An image with metadata.
 *
 *  This class implements shallow copying upon using the copy constructor or
 *  the assignment operator; in this sense, it behaves somewhat like a
 *  reference -- copying is cheap, but copies share the data with the originals.
 *  To make a deep copy, use @a clone or @a makeUnique.
 */
template <class T>
class GenericImage {
 public:
  template <class T2>
  friend class GenericImage;

  /// Type used to select either the image data, or the metadata, or both.
  enum SelectType { SEL_NONE = 0, SEL_IMAGE, SEL_META, SEL_BOTH };
  /// Type of elements stored by the image.
  typedef typename ImageBuffer<T>::value_type value_type;

  /// @name Members related to the image data.
  //@{

  /// Get direct access to the image data (read-only).
  const T* getData() const { return image_.getData(); }
  /// Get direct access to the image data.
  T* getData() { return image_.getData(); }

  /** @brief Access a given pixel in the data (read-only).
   *
   *  This returns a pointer to the first color component of the pixel.
   */
  const T* operator()(size_t x, size_t y) const { return image_(x, y); }
  /** @brief Access a given pixel in the data.
   *
   *  This returns a pointer to the first color component of the pixel.
   */
  T* operator()(size_t x, size_t y) { return image_(x, y); }

  /// Get width in pixels.
  size_t getWidth() const { return image_.getWidth(); }
  /// Get height in pixels.
  size_t getHeight() const { return image_.getHeight(); }
  /// Get the number of color channels in the image.
  size_t getChannelCount() const { return image_.getChannelCount(); }

  /** @brief Change image size -- this should only be used on images with no
   *         image data.
   *
   *  The correct amount of memory will be allocated next time @a allocate is
   *  used. If the image is not empty, an exception is thrown.
   */
  void reshape(size_t width, size_t height) { image_.reshape(width, height); }
  /** @brief Change the number of color channels.
   *
   *  When no image data is present, this sets the number of components to
   *  use on the next @a allocate. When used with existing image data, it
   *  restricts to the first @a n color channels. Note that this does not
   *  actually change the data in the image at all; to do that, use @a flatten.
   *  Also, this has undefined consequences if @a n is larger than the number
   *  of channels existing in the image data.
   */
  void setChannelCount(size_t n) { image_.setChannelCount(n); }

  /** @brief Make sure the image data is contiguous.
   *
   *  If the image data is already contiguous and in row-major order, nothing
   *  happens. Otherwise, the image is put in that form.
   */
  void flatten() { image_.flatten(); }

  /** @brief Crop the image.
   *
   *  @see ImageBuffer::crop.
   */
  void crop(size_t offsetX, size_t offsetY, size_t width=0, size_t height=0)
    { image_.crop(offsetX, offsetY, width, height); }

  /** @brief Select a channel in the image.
   *
   *  @see ImageBuffer::selectChannel.
   */
  void selectChannel(size_t i)
    { image_.selectChannel(i); image_.channel_types_ = 'k'; }

  /** @brief Return a cropped version of the image.
   *
   *  The metadata is (shallow-)copied as is.
   *
   *  @see ImageBuffer::cropped.
   */
  GenericImage<T> cropped(size_t offX, size_t offY, size_t w = 0, size_t h = 0)
    { GenericImage<T> res(*this); res.crop(offX, offY, w, h); return res; }

  /** @brief Get a grayscale image corresponding to one of the channels.
   *
   *  The metadata is (shallow-)copied as is.
   *
   *  @see ImageBuffer::separateChannel.
   */
  GenericImage<T> separateChannel(size_t i)
    { GenericImage<T> res(*this); res.selectChannel(i); return res; }

  /** @brief Rotate the image clock-wise by a multiple of 90 degrees.
   *
   *  @see ImageBuffer::coarseRotate.
   */
  void coarseRotate(int nRot) { image_.coarseRotate(nRot); }

  /** @brief Flip the image along one of the axes.
   *
   *  @see ImageBuffer::flip.
   */
  void flip(ImageAxis axis) { image_.flip(axis); }

  /** @brief Flip x and y axes.
   *
   *  @see ImageBuffer:flipXY.
   */
  void flipXY() { image_.flipXY(); }

  //@}

  /// @name Members related to metadata.
  //@{
  
  /// Access the metadata directly (read-only).
  const Metadata::Contents& getMetadata() const
    { return metadata_.getContents(); }

  /// Add a metadatum. This replaces pre-existing metadata with that name.
  void addMetadatum(const std::string& tag, const Metadatum& datum)
    { metadata_.getContents()[tag] = datum; }
  /// Append metadata to pre-existing tag. The IDs must match.
  void appendMetadatum(const std::string& tag, const Metadatum& datum);
  /// Get metadata (read-only).
  const Metadatum& getMetadatum(const std::string& tag) const;
  /// Get metadata.
  Metadatum& getMetadatum(const std::string& tag);
  /// Delete metadata.
  void removeMetadatum(const std::string& tag)
    { metadata_.getContents().erase(tag); }
  /// Check whether a given metadata tag exists.
  bool hasMetadatum(const std::string& tag) const {
    Metadata::Contents metaMap = metadata_.getContents();
    return metaMap.find(tag) != metaMap.end();
  }
  /// Copy metadata from a different image. Note that this is a shallow copy.
  template <class U>
  void copyMetadataFrom(const GenericImage<U>& original)
    { metadata_ = original.metadata_; }

  //@}

  ///@ name Members acting on both image data and metadata.
  //{@

  /** @brief Make a deep copy of the image.
   *
   *  This makes deep copies of both the image data and the metadata.
   */
  GenericImage<T> clone() const
    { GenericImage<T> result(*this); result.makeUnique(); return result; }

  /** @brief Make sure this image does not share memory with any other.
   *
   *  If the reference count of the image data (and/or metadata) is different
   *  from 1, a copy of all the data is made; otherwise, nothing happens.
   *  This also has the effect that after the call, the image data is
   *  contiguous and in row-major order.
   *
   *  By default, this calls @a makeUnique for both the image data and the
   *  metadata. Use the @a which parameter to change this behavior.
   */
  void makeUnique(SelectType which = SEL_BOTH) {
    if (which & SEL_IMAGE) image_.makeUnique();
    if (which & SEL_META) metadata_.makeUnique();
  }
  /** @brief Check whether the image is empty.
   *
   *  By default, this means that both the image data and the metadata are
   *  empty. Use the @a which parameter to change this behavior.
   */
  bool isEmpty(SelectType which = SEL_BOTH) const {
    bool res = true;
    if (which & SEL_IMAGE) res = (res && image_.isEmpty());
    if (which & SEL_META) res = (res && metadata_.getContents().empty());
    return res;
  }
  /** @brief Returns @a true if the object doesn't share its data with any
   *         other.
   *
   *  By default, this means that both the image data and the metadata are
   *  unique. This behavior can be changed by using the @a which parameter.
   */
  bool isUnique(SelectType which = SEL_BOTH) const {
    bool res = true;
    if (which & SEL_IMAGE) res = (res && image_.isUnique());
    if (which & SEL_META) res = (res && metadata_.isUnique());
    return res;
  }
  /** @brief Clear the image.
   *
   *  By default, this clears both the image data and the metadata. Use @a which
   *  to change this behavior.
   */
  void clear(SelectType which = SEL_BOTH) {
    if (which & SEL_IMAGE) image_.clear();
    if (which & SEL_META) metadata_.getContents().clear();
  }

  /// Get the total image data size.
  size_t getSize() { return image_.getSize(); }

  /** @brief Allocate space for a new image.
   *
   *  This clears the image data if it is not already empty. It has no effect on
   *  the metadata.
   */
  void allocate() { image_.allocate(); }
  //@}

  /// @name Other members.
  //@{

  /** @brief Get a string describing the types of the color channels.
   *
   *  @see setChannelTypes.
   */
  const std::string& getChannelTypes() const { return channel_types_; }
  /** @brief Set the types of the color channels.
   *
   *  This automatically sets the number of channels used by the image.
   *
   *  Each character in @a s represents the type of one channel, potentially
   *  in a context-dependent way. The @a GenericImage itself does not use this
   *  information in any way, but here is a list of recommended channel type
   *  codes:
   *   - '-' -- unused (padding)
   *   - 'a' -- alpha (in rgba), color component a (in Lab)
   *   - 'b' -- blue (in rgba), color component b (in Lab)
   *   - 'c' -- cyan
   *   - 'C' -- Cr or Cb (in YCC, depending on position)
   *   - 'g' -- green
   *   - 'k' -- black
   *   - 'L' -- lightness
   *   - 'm' -- magenta
   *   - 'r' -- red
   *   - 'X' -- color component X (in XYZ)
   *   - 'y' -- yellow
   *   - 'Y' -- luma (in YCC), color component Y (in XYZ)
   *   - 'Z' -- color component Z (in XYZ)
   */
  void setChannelTypes(const std::string& s)
    { image_.setChannelCount(s.length()); channel_types_ = s; }

  /// Clamp color to limits given by the color representation.
  static T clampColor(double x) {
    const T min = std::numeric_limits<T>::min();
    const T max = std::numeric_limits<T>::max();
    return (x < min)?min:((x > max)?max:x);
  }

  //@}

 private:
  ImageBuffer<T>      image_;
  Metadata            metadata_;
  std::string         channel_types_;
};

#endif
