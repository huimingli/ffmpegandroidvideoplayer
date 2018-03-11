add_library(PacketQueue SHARED PacketQueue.cpp)


target_link_libraries(PacketQueue avcodec)