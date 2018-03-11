add_library(Media SHARED Media.cpp)
target_link_libraries(Media avformat avutil Audio Video)