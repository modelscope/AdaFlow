cmake_minimum_required(VERSION 3.0)

project(xstreamer VERSION 1.0.1)

set(CMAKE_EXE_LINKER_FLAGS "-ldl -rdynamic")

set(CMAKE_BUILD_TYPE Release)

include_directories(
        ./
        ../
        ../../
        ../../../../3rdparty/json/include
        /adaflow-install/include/glib-2.0 #for GLib Library
        /adaflow-install/lib64/glib-2.0/include
        /adaflow-install/lib/x86_64-linux-gnu/glib-2.0/include #for GLib Library
        /adaflow-install/include/gstreamer-1.0 #for Gstreamer Library
        /adaflow-install/local/include/gstreamer-1.0 #for Gstreamer Library
        /adaflow-install/local/lib/x86_64-linux-gnu/gstreamer-1.0/include #for Gstreamer Library
)

add_library(test_subplugin SHARED flow_filter_template.cc)
add_library(yolodetect_subplugin SHARED flow_filter_yolodetect.cc)