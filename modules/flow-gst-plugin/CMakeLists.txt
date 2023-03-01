cmake_minimum_required(VERSION 3.22.1)
project(flow-gst-plugin VERSION 1.0.1)

# search for external dependencies
find_package(PkgConfig)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GST REQUIRED gstreamer-1.0)
pkg_search_module(GST_BASE REQUIRED gstreamer-base-1.0)
pkg_search_module(GST_APP REQUIRED gstreamer-app-1.0)
pkg_search_module(GST_VIDEO REQUIRED gstreamer-video-1.0)
pkg_search_module(GST_AUDIO REQUIRED gstreamer-audio-1.0)

set(FLOW_GST_BACKEND_TARGET_NAMES ${FLOW_GST_BACKEND_CORE_DY_LIB_NAME})
set(FLOW_GST_BACKEND_SOURCE_FILES elements/flow_python_extension.h elements/flow_python_extension.c)

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
            ${GLIB_INCLUDE_DIRS})


    # link libraries AND their dependencies
    target_link_libraries(${TARGET_NAME}
            ${GST_LIBRARIES}
            ${GST_BASE_LIBRARIES}
            ${GST_VIDEO_LIBRARIES}
            ${GST_AUDIO_LIBRARIES}
            ${GST_APP_LIBRARIES}
            ${GLIB_LIBRARIES})

    install(TARGETS ${TARGET_NAME}
            EXPORT ${TARGET_NAME}_TARGETS
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/gstreamer-1.0
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
            PUBLIC_HEADER DESTINATION include
            )

endforeach()