cmake_minimum_required(VERSION 3.12)

project(JumaSC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(JUMASC_ENABLE_COMPILE_TO_SPV "Build with glslang" OFF)
option(JUMASC_ENABLE_COMPILE_FROM_SPV "Build with SPIRV-Cross" OFF)
option(JUMASC_ENABLE_COMPILE_HLSL "Build with DXC compiler" OFF)

# glslang --------------------------------

if(JUMASC_ENABLE_COMPILE_TO_SPV)
    find_package(glslang REQUIRED)

    list(APPEND JUMASC_DEFINITIONS JUMASC_ENABLE_GLSLANG)
    list(APPEND JUMASC_LIBS glslang SPIRV MachineIndependent OSDependent GenericCodeGen OGLCompiler glslang-default-resource-limits)
endif()

# glslang --------------------------------

#find_package(spirv_cross_core REQUIRED)
#find_package(spirv_cross_glsl REQUIRED)
#find_package(spirv_cross_cpp REQUIRED)

## DXC ------------------------------------

if(JUMASC_ENABLE_COMPILE_HLSL)
    find_path(dxc_INCLUDE_DIR NAMES dxc/dxcapi.h REQUIRED)

    find_package(Vulkan COMPONENTS dxc)
    if(Vulkan_dxc_lib_FOUND)
        set(dxc_LIBRARY ${Vulkan_dxc_LIBRARY} CACHE FILEPATH "")
    else()
        find_library(dxc_LIBRARY NAMES dxcompiler REQUIRED)
    endif()

    list(APPEND JUMASC_DEFINITIONS JUMASC_ENABLE_DXC)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND JUMASC_COMPILE_OPTIONS -fms-extensions)
    endif()
    list(APPEND JUMASC_INCULE_DIRS ${dxc_INCLUDE_DIR})
    list(APPEND JUMASC_LIBS ${dxc_LIBRARY})
endif()

## DXC ------------------------------------

if(CMAKE_BUILD_TYPE MATCHES Debug)
    list(APPEND JUMASC_DEFINITIONS JUMASC_DEBUG)
endif()

list(APPEND JUMASC_HEADER_FILES
    include/JumaSC/JumaSC.h
    src/CompilerInternal.h
    src/JumaSC_dxc.h
    src/JumaSC_glslang.h
)
list(APPEND JUMASC_SOURCE_FILES
    src/CompilerInternal.cpp
    src/JumaSC_dxc.cpp
    src/JumaSC_glslang.cpp
)

add_library(JumaSC STATIC ${JUMASC_SOURCE_FILES})
if(JUMASC_COMPILE_OPTIONS)
    target_compile_options(JumaSC PRIVATE ${JUMASC_COMPILE_OPTIONS})
endif()
if(JUMASC_DEFINITIONS)
    target_compile_definitions(JumaSC PRIVATE ${JUMASC_DEFINITIONS})
endif()

target_include_directories(JumaSC PUBLIC include)
if(JUMASC_INCULE_DIRS)
    target_include_directories(JumaSC PRIVATE ${JUMASC_INCULE_DIRS})
endif()

target_link_libraries(JumaSC PUBLIC jutils)
if(JUMASC_COMPILE_OPTIONS)
    target_link_libraries(JumaSC PRIVATE ${JUMASC_LIBS})
endif()
