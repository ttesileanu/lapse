/** @file shearer.h
 *  @brief Apply shear filter to image.
 */
#ifndef TRANSFORMS_SHEARER_H_
#define TRANSFORMS_SHEARER_H_

#include <algorithm>
#include <numeric>

#include "image/image.h"
#include "misc/callback.h"

/// Class that handles shearing of images.
template <class T>
class Shearer {
 public:
  enum Direction { HORIZONTAL, VERTICAL };
 
  /// Constructor.
  Shearer(Direction direction) : direction_(direction) {}
  /// Destructor.
  virtual ~Shearer() {}

  /** @brief Shear the image by the given factor, using the current settings,
   *         and making the center of the final image match the center of the
   *         origin image.
   *
   *  Note that, while this always makes an image with image data not shared
   *  with any other image, the metadata is shallow-copied from the original.
   */
  void apply(const GenericImage<T>& image, GenericImage<T>& result,
      float factor);
  
  /** @brief Shear the image by the given factor, using the current settings,
   *         and making the given point of the final image match the center of
   *         the origin image. Mapping is given by
   *            d_x - d_cx = o_x - o_cx + factor*(o_y - o_cy)
   *            d_y - d_cy = o_y - o_cy
   *         where d_x, d_y are destination coordinates; o_x, o_y are origin
   *         coordinates; and (o_cx, o_cy), (d_cx, d_cy) are the locations of
   *         the centers in origin and destination images, respectively.
   *
   *  Note that, while this always makes an image with image data not shared
   *  with any other image, the metadata is shallow-copied from the original.
   */
  void apply(const GenericImage<T>& image, GenericImage<T>& result,
      float factor, float dest_ctr_x, float dest_ctr_y);

 protected:
  Direction                     direction_;
};

#endif
