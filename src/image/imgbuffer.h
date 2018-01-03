/** @file imgbuffer.h
 *  @brief An image buffer featuring lazy copying.
 */
#ifndef IMAGE_IMGBUFFER_H_
#define IMAGE_IMGBUFFER_H_

#include <limits>
#include <stdexcept>

#include <boost/shared_array.hpp>

/// Axis enum.
enum ImageAxis { NO_AXIS = 0, X_AXIS, Y_AXIS, BOTH_AXES };

/** @brief Image buffer featuring lazy copying.
 *
 *  This is designed to allow for fast copying when read-only access is
 *  required. Copying (either by copy constructor or assignment operator) is
 *  always shallow. This means that this class effectively behaves like a
 *  reference to the data. To make a deep copy, the @a clone or @a makeUnique
 *  functions should be used.
 *
 *  This class is rather agnostic of the contents of the data buffer. It can
 *  be considered a matrix of values of type @a T.
 *
 *  This class is meant to help in the implementation of other classes, and
 *  not to be used on its own.
 */
template <class T>
class ImageBuffer {
 public:
  /// Type of elements stored in the class.
  typedef T value_type;

  /// Empty constructor.
  ImageBuffer() : ptr_(0), width_(0), height_(0), ncomps_(0)
    { strides_[0] = strides_[1] = 0; }

  /// Get direct access to the data (read-only).
  const T* getData() const { return data_.get(); }
  /// Get direct access to the data.
  T* getData() { return data_.get(); }
  /// Get read-only access to the strides.
  const int* getStrides() const { return strides_; }

  /** @brief Access a given pixel in the data (read-only).
   *
   *  This returns a pointer to the first color component of the pixel.
   */
  const T* operator()(size_t x, size_t y) const
    { return ptr_ + strides_[0]*x + strides_[1]*y; }
  /** @brief Access a given pixel in the data.
   *
   *  This returns a pointer to the first color component of the pixel.
   */
  T* operator()(size_t x, size_t y)
    { return ptr_ + strides_[0]*x + strides_[1]*y; }

  /// Get width in pixels.
  size_t getWidth() const { return width_; }
  /// Get height in pixels.
  size_t getHeight() const { return height_; }
  /// Get the number of color channels in the image.
  size_t getChannelCount() const { return ncomps_; }

  /** @brief Change image size -- this should be used only on empty images.
   *
   *  The correct amount of memory will be allocated next time @a allocate is
   *  used. If the image is not empty, an exception is thrown.
   */
  void reshape(size_t width, size_t height) {
    if (!isEmpty())
      throw std::runtime_error("Reshape on non-empty ImageBuffer.");
    width_ = width; height_ = height;
  }
  /** @brief Change the number of color channels.
   *
   *  When this is used on empty images, it sets the number of components to
   *  use on the next @a allocate. When used on a pre-existing image, it
   *  restricts to the first @a n color channels. Note that this does not
   *  actually change the data in the image at all; to do that, use @a flatten.
   *  Also, this has undefined consequences if @a n is larger than the number
   *  of channels existing in the image data.
   */
  void setChannelCount(size_t n) { ncomps_ = n; }

  /** @brief Make sure the image data is contiguous.
   *
   *  If the image data is already contiguous and in row-major order, nothing
   *  happens. Otherwise, the image is put in that form.
   */
  void flatten() {
    if (!isEmpty() && (strides_[0] != (int)ncomps_
        || strides_[1] != (int)(ncomps_*width_)))
      forceCopy_();
  }

  /// Make a deep copy of the image.
  ImageBuffer<T> clone() const
    { ImageBuffer<T> result(*this); result.makeUnique(); return result; }
  /** @brief Make sure this image does not share memory with any other.
   *
   *  If the image is empty, or the reference count of the data is 1, nothing
   *  is done. Otherwise a copy of the image data is made in a newly-allocated
   *  buffer. This also has the effect that the image data is contiguous and
   *  in row-major order after the call.
   */
  void makeUnique() { if (!isUnique()) forceCopy_(); }
  /// Returns @a true if the image contains no data.
  bool isEmpty() const { return ptr_ == 0; }
  /** @brief Returns @a true if the image doesn't share its data with any other.
   *
   *  This can be either because the image is empty, or because the reference
   *  count of the data pointer is 1.
   */
  bool isUnique() const { return isEmpty() || data_.unique(); }
  /// Clear all image data. This preserves the number of channels.
  void clear() { data_.reset(); ptr_ = 0; width_ = height_ = 0; }

  /// Get total image size.
  size_t getSize() { return ncomps_*width_*height_; }

  /** @brief Allocate space for a new image.
   *
   *  This clears the image if it is not already empty.
   */
  void allocate();

  /** @brief Crop the image.
   *
   *  This works by changing the strides and offset in the image. No data
   *  copying happens. This has two effects: 1) cropping is fast; 2) after
   *  cropping, changing the image still affects the original image data.
   *
   *  If either @a width or @a height is zero, the size in that dimension
   *  is set to whatever is left from the image after the offset.
   */
  void crop(size_t offsetX, size_t offsetY, size_t width=0, size_t height=0);
  /** @brief Make a crop of the image, and return as a new image.
   *
   *  Note that this only makes a shallow copy, i.e., the cropped result and
   *  the original image share data.
   *
   *  @see crop.
   */
  ImageBuffer<T> cropped(size_t offX, size_t offY, size_t w = 0, size_t h = 0)
    { ImageBuffer<T> res(*this); res.crop(offX, offY, w, h); return res; }

  /** @brief Rotate the image clock-wise, by multiples of 90 degrees.
   *
   *  This is very fast, as it only alters the strides and does not actually
   *  change the image data.
   */
  void coarseRotate(int nRot);

  /** @brief Flip the image along one of the axes.
   *
   *  This is very fast, as it only alters the strides and does not actually
   *  change the image data.
   */
  void flip(ImageAxis axis);
  
  /** @brief Flip x and y axes of the image.
   *
   *  This is very fast, as it only alters the strides and does not actually
   *  change the image data.
   */
  void flipXY();

  /** @brief Select a channel in the image.
   *
   *  This manipulates the strides and offset in the image, and does not make
   *  an actual copy of the data. This means that cropping is fast, and the
   *  image still points to the same image data.
   */
  void selectChannel(size_t i) { ptr_ += i; ncomps_ = 1; }
  /** @brief Get a grayscale image corresponding to one of the channels.
   *
   *  Note that this only makes a shallow copy, i.e., the result and the 
   *  original image share data.
   *
   *  @see selectChannel.
   */
  ImageBuffer<T> separateChannel(size_t i)
    { ImageBuffer<T> res(*this); res.selectChannel(i); return res; }

 private:
  /// Force a copy of the image data to be made.
  void forceCopy_();

  /// Reference-counted pointer to the data.
  boost::shared_array<T>    data_;
  /// Simple pointer to identify starting point in @a data_.
  T*                        ptr_;
  /** @brief Strides show how to reach a given pixel in the image.
   *
   *  Start of (x, y) element is at
   *  @code 
   *    ptr_ + x*strides_[0] + y*strides_[1]
   *  @endcode
   *
   *  The presence of non-trivial strides and @a ptr_ != data_.get() allows
   *  to easily take subsets of a picture without performing a copy.
   */
  int                       strides_[2];
  /// Number of pixels in the x direction.
  size_t                    width_;
  /// Number of pixels in the y direction.
  size_t                    height_;
  /// Number of color channels per pixel.
  size_t                    ncomps_;
};

#endif
