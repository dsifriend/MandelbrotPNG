cmake_minimum_required(VERSION 3.8)
project(MandelbrotPNG)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c)
add_executable(MandelbrotPNG ${SOURCE_FILES})

# Linking zlib Library for use in libpng
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(MandelbrotPNG ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

# Linking PNG Library
find_package(PNG REQUIRED)
if(PNG_FOUND)
    include_directories(${PNG_INCLUDE_DIRS})
    target_link_libraries(MandelbrotPNG ${PNG_LIBRARIES})
endif(PNG_FOUND)