#设置工程名称
PROJECT(FFmpegTest)
#自己定制
SET(TARGET ffmpegDemo)
SET(LIBS avformat avcodec avutil swscale swresample z bz2 iconv sdl2)

#设置CMAKE最小版本
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
#设置编译器
SET(CMAKE_CXX_COMPILER "g++")
#设置构建类型，及相应的编译选项
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

#设置执行文件输出目录
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

MESSAGE("++++++++++++++Start Build+++++++++++++++++")

#头文件搜索目录
#INCLUDE_DIRECTORIES(inc)
#链接库搜索路径
#LINK_DIRECTORIES(/System/Library/Frameworks/)

#寻找系统库
find_library(AUDIO_TOOLBOX AudioToolbox)
if (NOT AUDIO_TOOLBOX)
    message(FATAL_ERROR "AUDIO_TOOLBOX not found")
endif()

find_library(VIDEO_TOOLBOX VideoToolbox)
if (NOT VIDEO_TOOLBOX)
    message(FATAL_ERROR "VIDEO_TOOLBOX not found")
endif()

find_library(CORE_FOUNDATION CoreFoundation)
if (NOT CORE_FOUNDATION)
    message(FATAL_ERROR "CORE_FOUNDATION not found")
endif()

find_library(CORE_MEDIA CoreMedia)
if (NOT CORE_MEDIA)
    message(FATAL_ERROR "CORE_MEDIA not found")
endif()

find_library(CORE_VIDEO CoreVideo)
if (NOT CORE_VIDEO)
    message(FATAL_ERROR "CORE_VIDEO not found")
endif()

find_library(SECURITY Security)
if (NOT SECURITY)
    message(FATAL_ERROR "SECURITY not found")
endif()

#当前目录的文件
SET(SRC_LIST src/simplest_ffmpeg_decoder.cpp 
             src/simplest_ffmpeg_player_su.cpp
             src/main.cpp)

#生成可执行文件
ADD_EXECUTABLE(${TARGET} ${SRC_LIST})

#执行文件链接属性
TARGET_LINK_LIBRARIES(${TARGET} ${LIBS} 
                        ${AUDIO_TOOLBOX} 
                        ${VIDEO_TOOLBOX}
                        ${CORE_FOUNDATION} 
                        ${CORE_MEDIA}
                        ${CORE_VIDEO}
                        ${SECURITY})