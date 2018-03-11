add_library(Audio SHARED Audio.cpp)
target_link_libraries(Audio avformat swresample PacketQueue SDL2 sdlmainso avutil log)