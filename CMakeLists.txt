cmake_minimum_required(VERSION 3.22.1)
project(adaflow VERSION 0.0.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
cmake_policy(SET CMP0126 NEW)
cmake_policy(SET CMP0077 NEW)
include(GNUInstallDirs)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
        set(BUILD_ON_MACOSX_INTEL ON)
    else()
        set(BUILD_ON_MACOSX_APPLE ON)
    endif()
else()
    set(BUILD_ON_LINUX ON)
endif()


if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
        set(BUILD_ON_MACOSX_INTEL ON)
    else()
        set(BUILD_ON_MACOSX_APPLE ON)
    endif()
else()
    set(BUILD_ON_LINUX ON)
endif()

# set option
OPTION(ADAFLOW_USE_CUDA "Build with CUDA" ON)
OPTION(ADAFLOW_USE_TRT "Build with TRT" ON)


# setup compiler flags
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(STATUS "------- Linux -------")
    link_directories($ENV{CONDA_PREFIX}/lib64 $ENV{CONDA_PREFIX}/lib $ENV{ADAFLOW_PREFIX}/lib $ENV{ADAFLOW_PREFIX}/lib64 /usr/local/cuda/lib64)
    include_directories($ENV{CONDA_PREFIX}/include $ENV{ADAFLOW_PREFIX}/include /usr/local/cuda/include)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    if(BUILD_ON_MACOSX_APPLE)
        message(STATUS "------- Apple Silicon Mac -------")
    elseif(BUILD_ON_MACOSX_INTEL)
        message(STATUS "------- Intel Mac -------")
    endif()
    set(ADAFLOW_USE_CUDA OFF)
    set(ADAFLOW_USE_TRT OFF)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows") # IF(WIN32) or IF(MSVC)
    message(FATAL_ERROR "------- Windows is not supported right now -------")
elseif(CMAKE_SYSTEM_NAME MATCHES "Android")
endif()

# shared variables
set(FLOW_GST_BACKEND_CORE_DY_LIB_NAME flow_gst_plugin)
#set(FLOW_GST_BACKEND_STATIC_LIB_NAME flow_gst_backend_static)


# add sub-modules
add_subdirectory(modules/adaflow-gst-plugin)
add_subdirectory(3rdparty/json)

