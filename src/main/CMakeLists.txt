# lapse
add_executable(lapse lapse.cc processor.cc)
target_link_libraries(lapse jpegwrapper transforms effects exifprops)
target_link_libraries(lapse ${JPEG_LIBRARY})
target_link_libraries(lapse ${Boost_LIBRARIES})
target_link_libraries(lapse ${EXIV2_LIBRARIES})
target_link_libraries(lapse ${LCMS2_LIBRARIES})
