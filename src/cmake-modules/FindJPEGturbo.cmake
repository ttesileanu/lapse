# - Find JPEG turbo
# Find turbo JPEG includes and library
# This module defines
#  JPEGTURBO_INCLUDE_DIR, where to find jpeglib.h, turbojpeg.h etc.
#  JPEGTURBO_LIBRARIES, the libraries needed to use JPEG.
#  JPEGTURBO_FOUND, If false, do not try to use JPEGturbo.
# also defined, but not for general use are
#  JPEGTURBO_LIBRARY, where to find the JPEGturbo library.

# adapted from FindJPEG by T. Tesileanu (2012, 2018)

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

#FIND_PATH(JPEGTURBO_INCLUDE_DIR jpeglib.h PATHS /opt/libjpeg-turbo/include NO_DEFAULT_PATH)
FIND_PATH(JPEGTURBO_INCLUDE_DIR turbojpeg.h PATHS /opt/libjpeg-turbo/include /usr/local/opt/jpeg-turbo/include)

SET(JPEGTURBO_NAMES ${JPEGTURBO_NAMES} turbojpeg)
#FIND_LIBRARY(JPEGTURBO_LIBRARY NAMES ${JPEGTURBO_NAMES} PATHS /opt/libjpeg-turbo/lib NO_DEFAULT_PATH)
FIND_LIBRARY(JPEGTURBO_LIBRARY NAMES ${JPEGTURBO_NAMES} PATHS /opt/libjpeg-turbo/lib /usr/local/opt/jpeg-turbo/lib)

# handle the QUIETLY and REQUIRED arguments and set JPEG_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(${CMAKE_MODULE_PATH}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(JPEGTURBO DEFAULT_MSG JPEGTURBO_LIBRARY JPEGTURBO_INCLUDE_DIR)

IF(JPEGTURBO_FOUND)
  SET(JPEGTURBO_LIBRARIES ${JPEGTURBO_LIBRARY})
ENDIF(JPEGTURBO_FOUND)

MARK_AS_ADVANCED(JPEGTURBO_LIBRARY JPEGTURBO_INCLUDE_DIR )
