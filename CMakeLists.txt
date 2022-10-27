cmake_minimum_required(VERSION 3.23)
project(LCDController)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS "-Wno-unused-local-typedefs -Wno-unused-function -std=c++11 -rdynamic -D_GLIBCXX_USE_CXX11_ABI=0")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")

#打印信息
message(STATUS "build type:")
message(STATUS "CXX_FLAGS = ${CMAKE_CXX_FLAGS} ")

#设置库文件输出目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/libs)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libs)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src/ControllerIPCall)
add_subdirectory(src/LCDController)
add_subdirectory(src/utilswapper)