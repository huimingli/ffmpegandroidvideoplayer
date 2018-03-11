add_library(FrameQueue SHARED FrameQueue.cpp)


target_link_libraries(FrameQueue avcodec avformat avutil)