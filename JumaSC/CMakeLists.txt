cmake_minimum_required(VERSION 3.12)

project(JumaSC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## DXC ------------------------------------

find_path(dxc_INCLUDE_DIR NAMES dxc/dxcapi.h REQUIRED)

find_package(Vulkan COMPONENTS dxc)
if(Vulkan_dxc_lib_FOUND)
    set(dxc_LIBRARY ${Vulkan_dxc_LIBRARY} CACHE FILEPATH "")
else()
    find_library(dxc_LIBRARY NAMES dxcompiler REQUIRED)
endif()

## DXC ------------------------------------

list(APPEND JUMASC_HEADER_FILES
    include/JumaSC/JumaSC.h
)
list(APPEND JUMASC_SOURCE_FILES
    src/JumaSC.cpp
)

add_library(JumaSC STATIC ${JUMASC_SOURCE_FILES})
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(JumaSC PRIVATE -fms-extensions)
endif()
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(JumaSC PRIVATE JUMASC_DEBUG)
endif()
target_include_directories(JumaSC PUBLIC include ${dxc_INCLUDE_DIR})
target_link_libraries(JumaSC PUBLIC jutils ${dxc_LIBRARY})
