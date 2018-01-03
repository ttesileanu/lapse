add_library(jpegwrapper jpeg.cc imgio.cc)
target_link_libraries(jpegwrapper ${JPEG_LIBRARY})
