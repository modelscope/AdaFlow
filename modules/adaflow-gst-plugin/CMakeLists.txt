cmake_minimum_required(VERSION 3.22.1)
project(adaflow-gst-plugin VERSION 0.0.1)

# set cuda version
set(ADAFLOW_CUDA_VERSION "11.6" CACHE STRING "cuda runtime version of current system. defaults to 11.6")

# search for external dependencies
find_package(PkgConfig)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GST REQUIRED gstreamer-1.0)
pkg_search_module(GST_BASE REQUIRED gstreamer-base-1.0)
pkg_search_module(GST_APP REQUIRED gstreamer-app-1.0)
pkg_search_module(GST_VIDEO REQUIRED gstreamer-video-1.0)
pkg_search_module(GST_AUDIO REQUIRED gstreamer-audio-1.0)

set(FLOW_GST_BACKEND_TARGET_NAMES ${FLOW_GST_BACKEND_CORE_DY_LIB_NAME})
set(FLOW_GST_BACKEND_SOURCE_FILES
        elements/flow_video_aggregate.c
        elements/flow_video_aggregate.h
        elements/flow_tensor_aggregate.c
        elements/flow_tensor_aggregate.h
        register/adaflow.cpp
        elements/common.c
        elements/common.h
        metadata/flow_json_meta.h
        metadata/flow_json_meta.cc
        elements/meta_tensor.cpp
        elements/meta_tensor.h
        elements/flow_tensor_decode.cpp
        elements/flow_tensor_decode.h
        elements/flow_tensor_convert.h
        elements/flow_tensor_convert.c
        elements/flow_tensor_transform.h
        elements/flow_tensor_transform.c
        elements/flow_tensor_compositor.h
        elements/flow_tensor_compositor.c
        elements/flow_tensor_resize.h
        elements/flow_tensor_resize.cpp)

set(FLOW_CORE_LINK_LIBRARIES nlohmann_json::nlohmann_json)

# configure cuda
IF (ADAFLOW_USE_CUDA)
    message(STATUS "Enable CUDA")
    add_compile_definitions(ADAFLOW_USE_CUDA)
    pkg_search_module(CUDA REQUIRED cudart-${ADAFLOW_CUDA_VERSION})
    list(APPEND FLOW_CORE_LINK_LIBRARIES ${CUDA_LIBRARIES})
    list(APPEND FLOW_CORE_EXTRA_INCLUDE_DIRS ${CUDA_INCLUDE_DIRS})
ENDIF ()

# configure trt
IF (ADAFLOW_USE_TRT)
    message(STATUS "Enable TensorRT")
    add_compile_definitions(ADAFLOW_USE_TRT)
    list(APPEND FLOW_GST_BACKEND_SOURCE_FILES ann/trt/trt_infer.cpp ann/trt/trt_infer.h)
    list(APPEND FLOW_GST_BACKEND_SOURCE_FILES elements/flow_trt_infer.cpp elements/flow_trt_infer.h)
    list(APPEND FLOW_CORE_LINK_LIBRARIES libnvinfer.so libnvonnxparser.so)
ENDIF ()

add_library(${FLOW_GST_BACKEND_TARGET_NAMES} SHARED ${FLOW_GST_BACKEND_SOURCE_FILES})
foreach(TARGET_NAME ${FLOW_GST_BACKEND_TARGET_NAMES})
    # add include dirs of local source files
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    # add system dependencies
    target_include_directories(${TARGET_NAME}  SYSTEM PUBLIC
            ${GST_INCLUDE_DIRS}
            ${GST_BASE_INCLUDE_DIRS}
            ${GST_VIDEO_INCLUDE_DIRS}
            ${GST_AUDIO_INCLUDE_DIRS}
            ${GST_APP_INCLUDE_DIRS}
            ${GLIB_INCLUDE_DIRS}
            ${FLOW_CORE_EXTRA_INCLUDE_DIRS})


    # link libraries AND their dependencies
    target_link_libraries(${TARGET_NAME}
            ${GST_LIBRARIES}
            ${GST_BASE_LIBRARIES}
            ${GST_VIDEO_LIBRARIES}
            ${GST_AUDIO_LIBRARIES}
            ${GST_APP_LIBRARIES}
            ${GLIB_LIBRARIES}
            ${FLOW_CORE_LINK_LIBRARIES})

    install(TARGETS ${TARGET_NAME}
            EXPORT ${TARGET_NAME}_TARGETS
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/gstreamer-1.0
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/gstreamer-1.0
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
            PUBLIC_HEADER DESTINATION include
            )

endforeach()


# install gst plugins written in python
set(PYTHON_PLUGIN_SRCS python/flow_metadata_sink.py python/flow_modelscope_pipeline.py python/flow_python_extension.py python/flow_python_metadata_aggregate.py python/flow_c2python.py)
install(FILES ${PYTHON_PLUGIN_SRCS} DESTINATION ${CMAKE_INSTALL_LIBDIR}/gstreamer-1.0/python)
