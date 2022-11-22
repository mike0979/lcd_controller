cmake_minimum_required(VERSION 3.23)
project(LCDController)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS "-Wno-unused-local-typedefs -Wno-unused-function -std=c++11 -rdynamic -D_GLIBCXX_USE_CXX11_ABI=0 -DRAPIDJSON_HAS_STDSTRING=1")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")

#设置库文件输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src/ControllerIPCall)
add_subdirectory(src/LCDController)
add_subdirectory(src/utilswapper)