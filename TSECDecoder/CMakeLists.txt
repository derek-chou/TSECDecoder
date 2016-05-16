cmake_minimum_required (VERSION 2.6)

project(TSECDecoder)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(LibUV)
include_directories(SYSTEM ${LIBUV_INCLUDE_DIRS})
find_package(Iconv)
include_directories(SYSTEM ${ICONV_INCLUDE_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# Directories
aux_source_directory(./ SRC_LIST)
aux_source_directory(./hiredis REDIS_SRC)
aux_source_directory(./libhl LIBHL_SRC)
list(APPEND SRC_LIST ${REDIS_SRC} ${LIBHL_SRC})

# Release mode
set(CMAKE_BUILD_TYPE Debug)

# Compile flags
set(CMAKE_C_FLAGS "-Werror")

# I like verbose, must after project, do not know why
set(CMAKE_VERBOSE_MAKEFILE true)

# Where to include?
include_directories(${PROJECT_SOURCE_DIR}/hiredis;${PROJECT_SOURCE_DIR}/libhl;${LIBUV_INCLUDE_DIRS})


# Files to compile
add_executable(TSECDecoder ${SRC_LIST})

set_property(TARGET TSECDecoder PROPERTY C_STANDARD 99)

find_library(LIBUV_LIBRARIES ${LIBUV_INCLUDE_DIRS} ./)
target_link_libraries(TSECDecoder PUBLIC ${LIBUV_LIBRARIES})

find_library(ICONV_LIBRARIES ./)
target_link_libraries(TSECDecoder PUBLIC ${ICONV_LIBRARIES})
