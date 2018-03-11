add_library(Video SHARED Video.cpp)
target_link_libraries(Video swresample swscale PacketQueue FrameQueue)