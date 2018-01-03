add_library(effects exposure.cc effectfactory.cc whitebalance.cc cropresize.cc pad.cc)
target_link_libraries(effects ${EXIV2_LIBRARIES})
target_link_libraries(effects ${LCMS2_LIBRARIES})
