// large parts of this code are simply "paraphrased" from ImageMagick

#include "jpeg.h"

#include <algorithm>
#include <stdexcept>

#include <cmath>

#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>

#include "image/metadata.h"
#include "image/image-impl.h"
#include "misc/endian.h"

typedef JpegIO::Image Image;

// some JPEG marker constants
const unsigned EXIF_MARKER = JPEG_APP0 + 1;
const unsigned XMP_MARKER = JPEG_APP0 + 1;
const unsigned ICC_MARKER = JPEG_APP0 + 2;
const unsigned IPTC_MARKER = JPEG_APP0 + 13;

struct cErrorManager {
  jpeg_error_mgr    pub;            // "public" fields
};
typedef cErrorManager* cErrorManagerPtr;

static void cErrorExit(j_common_ptr cinfo)
{
  char errText[JMSG_LENGTH_MAX];
  cinfo -> err -> format_message(cinfo, errText);

  throw std::runtime_error(errText);
}

static int jpegGetCharacter(j_decompress_ptr pstatus)
{
  if (pstatus -> src -> bytes_in_buffer == 0)
    (*pstatus -> src -> fill_input_buffer)(pstatus);
  --pstatus -> src -> bytes_in_buffer;

  return GETJOCTET(*pstatus -> src -> next_input_byte++);
}

static unsigned jpegReadUint16(j_decompress_ptr pstatus)
{
  unsigned x = ((unsigned)jpegGetCharacter(pstatus)) << 8;
  x += jpegGetCharacter(pstatus);

  return x;
}

static std::string jpegReadString(j_decompress_ptr pstatus, size_t length)
{
  std::string result;
  result.reserve(length);
  for (size_t i = 0; i < length; ++i) {
    result += (char)jpegGetCharacter(pstatus);
  }

  return result;
}

static Blob jpegReadBlob(j_decompress_ptr pstatus, size_t length)
{
  Blob result;
  result.reserve(length);
  for (size_t i = 0; i < length; ++i) {
    result.push_back((unsigned char)jpegGetCharacter(pstatus));
  }

  return result;
}

static size_t jpegSkip(j_decompress_ptr pstatus, size_t n)
{
  for (size_t i = 0; i < n; ++i)
    jpegGetCharacter(pstatus);

  return n;
}

static boolean readComment(j_decompress_ptr pstatus)
{
  // read length of comment
  size_t length = jpegReadUint16(pstatus);

  // length < 2 should really never happen, but just in case...
  if (length <= 2)
    return true;

  // the remaining length
  length -= 2;

  // read the comment
  Image* image = static_cast<Image*>(pstatus -> client_data);
  image -> appendMetadatum("comment", Metadatum(jpegReadBlob(pstatus, length)));

  return true;
}

static boolean readIptcProfile(j_decompress_ptr pstatus)
{
  // read length of profile
  size_t length = jpegReadUint16(pstatus);

  // length < 2 should really never happen, but just in case...
  if (length <= 2)
    return true;

  // the remaining length
  length -= 2;

  // if this thing is too short to contain anything interesting, skip it!
  if (length <= 14) {
    // XXX maybe issue some warning?
    jpegSkip(pstatus, length);

    return true;
  }

  // check that this has the right magic word
  std::string word = jpegReadString(pstatus, 10);
  length -= 10; // remaining length
  if (word != "Photoshop ") {
    // not an IPTC profile, skip it
    // XXX maybe issue some warning?
    jpegSkip(pstatus, length);

    return true;
  }

  // read version number
  word += jpegReadString(pstatus, 4);
  length -= 4;

  // get access to the image
  Image* image = static_cast<Image*>(pstatus -> client_data);
  image -> appendMetadatum("iptc",
    Metadatum(jpegReadBlob(pstatus, length), word));

  return true;
}

static boolean readColorProfile(j_decompress_ptr pstatus)
{
  // read length of profile
  size_t length = jpegReadUint16(pstatus);

  // length < 2 should really never happen, but just in case...
  if (length <= 2)
    return true;

  // the remaining length
  length -= 2;

  // if this thing is too short to contain anything interesting, skip it!
  if (length <= 14) {
    // XXX maybe issue some warning?
    jpegSkip(pstatus, length);

    return true;
  }

  // check that this has the right magic word
  std::string word = jpegReadString(pstatus, 12);
  length -= 12; // remaining length
  // can't compare to "ICC_PROFILE\0" because that's interpreted as a C string,
  // and C strings end at \0, they don't contain them...
  if ((int)word[word.length()-1] != 0 || word.substr(0, 11) != "ICC_PROFILE") {
    // XXX maybe issue some warning?
    // not an ICC profile, skip it
    jpegSkip(pstatus, length);

    return true;
  }

  // skip current chunk id and total number of chunks
  length -= jpegSkip(pstatus, 2);

  // get access to the image
  Image* image = static_cast<Image*>(pstatus -> client_data);
  image -> appendMetadatum("icc", Metadatum(jpegReadBlob(pstatus, length),
    word));

  return true;
}

static std::string stringFromBlob(const Blob& blob, size_t length = 0)
{
  const char* p = reinterpret_cast<const char*>(&blob.front());
  if (length == 0)
    length = blob.size();

  return std::string(p, p + length);
}

static boolean readOtherProfile(j_decompress_ptr pstatus)
{
  // read length of profile
  size_t length = jpegReadUint16(pstatus);

  // length < 2 should really never happen, but just in case...
  if (length <= 2)
    return true;

  // the remaining length
  length -= 2;

  unsigned marker = pstatus -> unread_marker;

  Metadatum meta(jpegReadBlob(pstatus, length));

  std::string name;

  // check for a magic word -- could be exif or XMP
  if ((marker == EXIF_MARKER || marker == XMP_MARKER) && length >= 4) {
    std::string checkexif = stringFromBlob(meta.blob, 4);
    std::transform(checkexif.begin(), checkexif.end(), checkexif.begin(),
      ::tolower);
    if (marker == EXIF_MARKER && checkexif == "exif") {
      name = "exif";
      if (length >= 6) {
        meta.id = stringFromBlob(meta.blob, 6);
        meta.blob.erase(meta.blob.begin(), meta.blob.begin() + 6);
      } else {
        return true;
      }
    } else if (marker == XMP_MARKER && length >= 5) {
      std::string checkxmp = stringFromBlob(meta.blob, 5);
      std::transform(checkxmp.begin(), checkxmp.end(), checkxmp.begin(),
        ::tolower);
      if (checkxmp == "http:") {
        name = "xmp";
        std::string fullid = "";
        for (size_t i = 0; i < meta.blob.size(); ++i) {
          const unsigned char ch = meta.blob[i];
          fullid += ch;
          if (ch == '\0') {
            meta.blob.erase(meta.blob.begin(), meta.blob.begin() + i + 1);
            break;
          }
        }
        meta.id = fullid;
      }
    }
  }

  if (name.empty()) {
    // define a generic name
    name = "jpeg_app" + boost::lexical_cast<std::string>(marker - JPEG_APP0);
  }

  // get access to the image
  Image* image = static_cast<Image*>(pstatus -> client_data);
  image -> appendMetadatum(name, meta);

  return true;
}

static void writeComment(j_compress_ptr pstatus, const Image& image)
{
  if (!image.hasMetadatum("comment"))
    return; // nothing to do

  const Blob& commentBlob = image.getMetadatum("comment").blob;

  for (size_t i = 0; i < commentBlob.size(); i += 65533) {
    size_t length = std::min(commentBlob.size() - i, (size_t)65533);
    jpeg_write_marker(pstatus, JPEG_COM,
      (const JOCTET*)(&commentBlob[i]), length);
  }
}

static void writeOtherProfile(j_compress_ptr pstatus, const std::string& name,
    const Metadatum& meta)
{
  unsigned marker;
  
  if (name.substr(0, 8) == "jpeg_app")
    marker = JPEG_APP0 + boost::lexical_cast<unsigned>(name.substr(8));
  else if (name == "iptc")
    marker = IPTC_MARKER;
  else if (name == "exif")
    marker = EXIF_MARKER;
  else if (name == "icc") // this should never be called for icc
    marker = ICC_MARKER;
  else if (name == "xmp")
    marker = XMP_MARKER;
  else // unrecognized metadata, skip it
    return;
  
  size_t idLen = meta.id.length();
  size_t chunkLen = 65533 - idLen;
  if (meta.id.empty()) {
    for (size_t i = 0; i < meta.blob.size(); i += chunkLen) {
      size_t len = std::min(meta.blob.size() - i, (size_t)chunkLen);
      jpeg_write_marker(pstatus, marker, &meta.blob[i], len);
    }
  } else {
    std::vector<unsigned char> buffer(65535);
    for (size_t pos = 0; pos < meta.blob.size(); pos += chunkLen) {
      std::copy(meta.id.begin(), meta.id.end(), buffer.begin());

      size_t length = std::min(meta.blob.size() - pos, (size_t)chunkLen);
      // actual data
      std::copy(meta.blob.begin() + pos, meta.blob.begin() + pos + length,
        buffer.begin() + idLen);
      jpeg_write_marker(pstatus, marker, &buffer[0], length + idLen);
    }
  }
}

static void writeColorProfile(j_compress_ptr pstatus, const Image& image)
{
  // this should only be called if image has an ICC profile
  const Metadatum& meta = image.getMetadatum("icc");
  if (meta.blob.empty()) // nothing to do
    return;

  size_t idLen = meta.id.length();
  size_t chunkLen = 65533 - idLen - 2;
  size_t nChunks = (meta.blob.size() - 1) / chunkLen + 1;

  std::vector<unsigned char> buffer(65535);
  size_t pos = 0;
  for (size_t i = 0; i < nChunks; ++i, pos += chunkLen) {
    std::copy(meta.id.begin(), meta.id.end(), buffer.begin());
    // chunk id and total number of chunks
    buffer[idLen] = i + 1;
    buffer[idLen + 1] = nChunks;

    size_t length = std::min(meta.blob.size() - pos, (size_t)chunkLen);
    // actual data
    std::copy(meta.blob.begin() + pos, meta.blob.begin() + pos + length,
      buffer.begin() + idLen + 2);
    jpeg_write_marker(pstatus, ICC_MARKER, &buffer[0], length + idLen + 2);
  }
}

static void writeProfiles(j_compress_ptr pstatus, const Image& image)
{
  if (image.hasMetadatum("icc"))
    writeColorProfile(pstatus, image);

  // write all other profiles
  const Metadata::Contents& allMeta = image.getMetadata();
  for (Metadata::Contents::const_iterator i = allMeta.begin();
       i != allMeta.end(); ++i)
  {
    if (i -> first == "icc")
      continue; // we've done this already

    writeOtherProfile(pstatus, i -> first, i -> second);
  }
}

std::string JpegIO::convertColorspace(J_COLOR_SPACE space)
{
  std::string result;
  switch (space) {
    case JCS_GRAYSCALE:
      result = "k";
      break;
    case JCS_RGB:
    case JCS_EXT_RGB:
      result = "rgb";
      break;
    case JCS_EXT_BGR:
      result = "bgr";
      break;
    case JCS_YCbCr:
      result = "YCC";
      break;
    case JCS_CMYK:
      result = "cmyk";
      break;
    case JCS_YCCK:
      result = "YCCk";
      break;
    default:
      throw std::runtime_error("[JpegIO] Unrecognized color space.");
  }

  return result;
}

static endian::ByteOrder findExifByteOrder(const Blob& exif)
{
  if (exif.size() < 8 || exif[0] != exif[1])
    return endian::BO_UNKNOWN;

  if (exif[0] == 0x49)
    return endian::BO_LITTLE_ENDIAN;
  else if (exif[0] == 0x4D)
    return endian::BO_BIG_ENDIAN;
  else
    return endian::BO_UNKNOWN;
}

static const uint16_t* findExifOrientationConstPtr(const Blob& exif,
    endian::ByteOrder bo)
{
  size_t size = exif.size();
  if (size < 8)
    return 0;
  const unsigned char* p = &exif.front();

  uint32_t ifdOffset = toNative(*reinterpret_cast<const uint32_t*>(p + 4), bo);

  if (size < ifdOffset + 2)
    return 0;
  p += ifdOffset;
  uint16_t nIfds = toNative(*reinterpret_cast<const uint16_t*>(p), bo);

  if (size < ifdOffset + 2 + 12*nIfds)
    return 0;
  p += 2;
  for (uint16_t i = 0; i < nIfds; ++i) {
    uint16_t tag = toNative(*reinterpret_cast<const uint16_t*>(p), bo);
    if (tag == 0x0112) {
      // this is the orientation tag!
      uint16_t type = toNative(*reinterpret_cast<const uint16_t*>(p + 2), bo);
      if (type != 3) // unrecognized type for orientation tag
        return 0;

      uint32_t comps = toNative(*reinterpret_cast<const uint32_t*>(p + 4), bo);
      if (comps != 1) // orientation tag should have only one component
        return 0;

      return reinterpret_cast<const uint16_t*>(p + 8);
    }
    p += 12;
  }

  return 0;
}

static uint16_t* findExifOrientationPtr(Blob& exif, endian::ByteOrder bo)
{
  return const_cast<uint16_t*>(findExifOrientationConstPtr(exif, bo));
}

static uint16_t findExifOrientation(const Blob& exif, endian::ByteOrder bo)
{
  const uint16_t* p = findExifOrientationConstPtr(exif, bo);
  if (!p)
    return 1;
  else
    return *p;
}

JpegIO::Header JpegIO::inspect(const std::string& name) const
{
  Header result;

  // open file
  FILE* file = 0;

  if (!(file = fopen(name.c_str(), "rb")))
    throw std::runtime_error("[JpegIO::inspect]: Couldn't open file.");

  jpeg_decompress_struct status;
  cErrorManager jerr;

  // setup error handler
  status.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = cErrorExit;

  boost::scoped_ptr<Image>  image;
  if (obeyOrientationTag_) {
    // need an image to use the EXIF handler
    image.reset(new Image);
    // setup an image as "client data", to have access to some members
    status.client_data = &image;
  }

  // initialize decompression object
  jpeg_create_decompress(&status);
  jpeg_stdio_src(&status, file);

  if (obeyOrientationTag_) {
    // set EXIF handler
    jpeg_set_marker_processor(&status, EXIF_MARKER, readOtherProfile);
  }

  // read the header
  jpeg_read_header(&status, true);

  result.width = status.image_width;
  result.height = status.image_height;
  result.ncomps = status.num_components;
  result.colorspace = convertColorspace(status.jpeg_color_space);

  if (obeyOrientationTag_ && image -> hasMetadatum("exif")) {
    // parse EXIF...
    const Blob& exifBlob = image -> getMetadatum("exif").blob;

    endian::ByteOrder bo = findExifByteOrder(exifBlob);
    uint16_t orientation = findExifOrientation(exifBlob, bo);
    switch (orientation) {
      case 5:
      case 6:
      case 7:
      case 8:
        std::swap(result.width, result.height);
        break;
      case 1:
      case 2:
      case 3:
      case 4:
      default:;
    }
  }

  return result;
}

std::pair<size_t, size_t> JpegIO::processSizeHint_(size_t w, size_t h) const
{
  std::pair<size_t, size_t> result(1, 1);
  if (sizeHint_.first == 0 || sizeHint_.second == 0)
    return result;

  if (w >= sizeHint_.first && h >= sizeHint_.second) {
    const float scaleX = (float)w / sizeHint_.first;
    const float scaleY = (float)h / sizeHint_.second;
    const float scale = std::max(scaleX, scaleY);

    const size_t pow = std::min(8.0, std::floor(std::log(scale) / std::log(2)));

    // XXX newer versions of jpeglib can handle more fractions
    result.second = (1 << pow);
  }

  return result;
}

Image JpegIO::load(const std::string& name) const
{
  Image result;

  // open file
  FILE* file = 0;

  if (!(file = fopen(name.c_str(), "rb")))
    throw std::runtime_error("[JpegIO::load]: Couldn't open file.");

  jpeg_decompress_struct status;
  cErrorManager jerr;

  // setup error handler
  status.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = cErrorExit;

  // setup the image as "client data", to have access to some members
  status.client_data = &result;

  // initialize decompression object
  jpeg_create_decompress(&status);
  jpeg_stdio_src(&status, file);

  // setup some handlers for various metadata
  jpeg_set_marker_processor(&status, JPEG_COM, readComment);
  jpeg_set_marker_processor(&status, ICC_MARKER, readColorProfile);
  jpeg_set_marker_processor(&status, IPTC_MARKER, readIptcProfile);
  for (int i = 1; i < 16; ++i) {
    const unsigned j = JPEG_APP0 + i;
    if (j != JPEG_COM && j != ICC_MARKER && j != IPTC_MARKER)
      jpeg_set_marker_processor(&status, j, readOtherProfile);
  }

  // read the header
  jpeg_read_header(&status, true);

  // handle the size hint
  std::pair<size_t, size_t> scaleFraction = processSizeHint_
    (status.image_width, status.image_height);
  status.scale_num = scaleFraction.first;
  status.scale_denom = scaleFraction.second;

  // XXX there are lots of parameters in jpeg_decompress_struct that I don't
  // use/understand

  // start decompression
  jpeg_start_decompress(&status);

  result.reshape(status.output_width, status.output_height);
  result.setChannelCount(status.output_components);
  result.setChannelTypes(convertColorspace(status.out_color_space));
  if ((size_t)status.output_components != result.getChannelTypes().length())
    throw std::runtime_error("[JpegIO::load] Number of components does not "
      "match channel descriptions.");

  // allocate memory
  result.allocate();

  // how many rows to read at a time
  size_t rowstep = status.rec_outbuf_height;

  // the input to jpeg_read_scanlines is an array of pointers...
  JSAMPARRAY ptrs = new JSAMPROW[rowstep];

  // read!
  while (status.output_scanline < status.output_height) {
    for (size_t i = 0; i < rowstep; ++i) {
      ptrs[i] = result(0, status.output_scanline + i);
    }
    jpeg_read_scanlines(&status, ptrs, rowstep);

    if (!notifyCallback_(status.output_scanline, status.output_height))
      break; // XXX will this screw something up with the jpeg decompression?
  }

  // finish & clean up
  delete[] ptrs;

  jpeg_finish_decompress(&status);
  jpeg_destroy_decompress(&status);

  fclose(file);

  // handle the orientation, if we were asked to...
  if (obeyOrientationTag_ && result.hasMetadatum("exif")) {
    // parse EXIF...
    Blob& exifBlob = result.getMetadatum("exif").blob;

    endian::ByteOrder bo = findExifByteOrder(exifBlob);
    uint16_t* orientationPtr = findExifOrientationPtr(exifBlob, bo);

    if (orientationPtr) {
      uint16_t orientation = endian::toNative(*orientationPtr, bo);
      switch (orientation) {
        case 2:
          result.flip(X_AXIS);
          break;
        case 4:
          result.flip(X_AXIS);
          // ...and then rotate
        case 3:
          result.coarseRotate(2);
          break;
        case 5:
          result.flip(Y_AXIS);
          // ...and then rotate
        case 6:
          result.coarseRotate(1);
          break;
        case 7:
          result.flip(Y_AXIS);
          // ...and then rotate
        case 8:
          result.coarseRotate(-1);
          break;
        case 1: // 1 is normal orientation
        default:;
      }
      // if we've rotated at all, flatten the image, and update EXIF
      if (orientation > 1 && orientation <= 8) {
        result.flatten();
        *orientationPtr = fromNative(uint16_t(1), bo);
      }
    }
  }

  // notify the callback that we're done
  notifyCallback_(status.output_height, status.output_height);

  return result;
}

void JpegIO::write(const std::string& name, const Image& img0) const
{
  // open file
  FILE* file = 0;

  jpeg_compress_struct status;
  cErrorManager jerr;
  JSAMPARRAY ptrs = 0;

  // setup error handler
  status.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = cErrorExit;

  if (!(file = fopen(name.c_str(), "wb")))
    throw std::runtime_error("[JpegIO::write]: Couldn't open file.");

  // need a flat image to write to file
  Image img(img0);
  img.flatten();

  // setup the image as "client data", to have access to some members
  // the C API can't treat this as a const, need a const_cast...
  status.client_data = const_cast<Image*>(&img);

  // initialize decompression object
  jpeg_create_compress(&status);
  jpeg_stdio_dest(&status, file);

  // set compression parameters
  status.image_width = img.getWidth();
  status.image_height = img.getHeight();
  status.input_components = img.getChannelCount();
  
  const std::string& chTypes = img.getChannelTypes();
  if (chTypes == "k") {
    status.in_color_space = JCS_GRAYSCALE;
  } else if (chTypes == "rgb") {
    status.in_color_space = JCS_RGB;
  } else if (chTypes == "bgr") {
    status.in_color_space = JCS_EXT_BGR;
  } else if (chTypes == "YCC") {
    status.in_color_space = JCS_YCbCr;
  } else if (chTypes == "cmyk") {
    status.in_color_space = JCS_CMYK;
  } else if (chTypes == "YCCk") {
    status.in_color_space = JCS_YCCK;
  } else {
    throw std::runtime_error("[JpegIO::write] Unrecognized color space.");
  }

  // XXX have more settings
  jpeg_set_defaults(&status);
  jpeg_set_quality(&status, writeQuality_, true);

  // start compression
  jpeg_start_compress(&status, true);

  // write the comment to file
  writeComment(&status, img);

  // write the profiles to file
  writeProfiles(&status, img);

  // how many rows to write at a time
  size_t rowstep = 1;

  // the input to jpeg_write_scanlines is an array of pointers...
  ptrs = new JSAMPROW[rowstep];

  // write!
  while (status.next_scanline < status.image_height) {
    for (size_t i = 0; i < rowstep; ++i) {
      // C library has no "const", need to const cast...
      ptrs[i] = const_cast<JSAMPLE*>(img(0, status.next_scanline + i));
    }
    jpeg_write_scanlines(&status, ptrs, rowstep);

    if (!notifyCallback_(status.next_scanline, status.image_height))
      break;
  }

  // finish & clean up
  delete[] ptrs;

  jpeg_finish_compress(&status);
  jpeg_destroy_compress(&status);

  // close the input file
  fclose(file);

  // notify the callback that we're done
  notifyCallback_(status.image_height, status.image_height);
}
