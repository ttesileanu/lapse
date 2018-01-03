/** @file image-impl.h
 *  @brief Implementation of some functions in image.h
 */
#ifndef IMAGE_IMAGE_IMPL_H_
#define IMAGE_IMAGE_IMPL_H_

#include "image.h"
#include "imgbuffer-impl.h"

template <class T>
void GenericImage<T>::appendMetadatum(const std::string& tag,
    const Metadatum& datum)
{
  Metadata::Contents& metaMap = metadata_.getContents();
  Metadata::Contents::iterator i = metaMap.find(tag);
  if (i == metaMap.end()) {
    // this is a new tag
    addMetadatum(tag, datum);
  } else {
    // check that the IDs match
    if (datum.id != i -> second.id)
      throw std::runtime_error("Trying to combine metadata with different ID.");

    i -> second.blob.insert(i -> second.blob.end(), datum.blob.begin(),
      datum.blob.end());
  }
}

template <class T>
const Metadatum& GenericImage<T>::getMetadatum(const std::string& tag) const
{
  const Metadata::Contents& metaMap = metadata_.getContents();
  Metadata::Contents::const_iterator i = metaMap.find(tag);
  if (i == metaMap.end())
    throw std::runtime_error("Metadata not found [getMetadatum].");
  else
    return i -> second;
}

template <class T>
Metadatum& GenericImage<T>::getMetadatum(const std::string& tag)
{
  Metadata::Contents& metaMap = metadata_.getContents();
  Metadata::Contents::iterator i = metaMap.find(tag);
  if (i == metaMap.end())
    throw std::runtime_error("Metadata not found [getMetadatum].");
  else
    return i -> second;
}

#endif
