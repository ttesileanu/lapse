# cmake version
cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)

# project name
project(lapse)

# version
set(lapse_VERSION_MAJOR 0)
set(lapse_VERSION_MINOR 1)
set(lapse_VERSION_PATCH 0)

# place executables at the root of the binary tree
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# search for modules in the cmake folder
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake-modules)

# search for include files rooted at the project directory
include_directories(${CMAKE_SOURCE_DIR})

# flags for all build types
# XXX these are gcc/clang specific!
add_definitions(-Wall --pedantic -std=c++11)

# add profiling options if asked for
# XXX these are gcc specific!
if (PROFILING)
  add_definitions(-fprofile-arcs -ftest-coverage)
  set(CMAKE_LINK_FLAGS "{$CMAKE_LINK_FLAGS} -coverage")
endif()

# find boost libraries
# need 1.41 for property tree
find_package(Boost 1.41 REQUIRED COMPONENTS system thread filesystem program_options)

# find jpeg libraries
find_package(JPEGturbo)
if (NOT JPEGTURBO_FOUND)
  message(STATUS "JPEGturbo library not found, speed will not be optimal.")
  find_package(JPEG REQUIRED)
else()
  set(JPEG_INCLUDE_DIR ${JPEGTURBO_INCLUDE_DIR})
  set(JPEG_LIBRARY ${JPEGTURBO_LIBRARY})
  set(JPEG_LIBRARIES ${JPEGTURBO_LIBRARIES})
endif()

# find exiv2
find_package(Exiv2 REQUIRED)

# find lcms2
find_package(Lcms2 REQUIRED)

# add relevant paths
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${JPEG_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${EXIV2_INCLUDE_DIR})
include_directories(${LCMS2_INCLUDE_DIR})

# add subdirectories
add_subdirectory(main)
add_subdirectory(file)
add_subdirectory(transforms)
add_subdirectory(effects)
add_subdirectory(exifprops)
