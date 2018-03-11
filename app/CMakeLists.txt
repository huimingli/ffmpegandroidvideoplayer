# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.





include_directories(src/main/jni/sdl/include)
include_directories(src/main/jni/ffmpeg/include)
add_library( # Sets the name of the library.
             sdlmain

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/jni/sdl/src/main/android/SDL_android_main.cpp
             src/main/jni/SDL4Android.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

add_library( SDL2
             SHARED
             IMPORTED)
set_target_properties( SDL2
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jni/armeabi/libSDL2.so)

add_library( avcodec
             SHARED
             IMPORTED)
set_target_properties( avcodec
                       PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/src/main/jni/armeabi/libavcodec.so)
add_library( avfilter
             SHARED
             IMPORTED)
set_target_properties( avfilter
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jni/armeabi/libavfilter.so)


add_library( avutil
             SHARED
             IMPORTED)
set_target_properties( avutil
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jni/armeabi/libavutil.so)

add_library( swresample
             SHARED
             IMPORTED)
set_target_properties( swresample
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jni/armeabi/libswresample.so)

add_library( swscale
             SHARED
             IMPORTED)
set_target_properties( swscale
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jni/armeabi/libswscale.so)
add_library( avformat
             SHARED
             IMPORTED)
set_target_properties( avformat
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jni/armeabi/libavformat.so)

add_library( sdlmainso
             SHARED
             IMPORTED)
set_target_properties( sdlmainso
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jni/armeabi/libmain.so)
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/src/main/jni/FrameQueue)
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/src/main/jni/PacketQueue)
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/src/main/jni/Video)
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/src/main/jni/Audio)
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/src/main/jni/Media)
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/src/main/jni/ReadPacketsThread)


target_link_libraries( # Specifies the target library.
                       sdlmain
                       avutil
                       ReadPacketsThread
                       SDL2
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                        android
                        )
