# SubPlugin API

## Development Guide

### 1. Create the boilerplate code

To help create the boilerplate code quickly, there is a tool in the ./filter_plugins/tools/ directory. This tool, make_element, is a command line utility that creates the boilerplate code for you.

To use make_element, first open up a terminal window. Change to the ./filter_plugins/tools/ directory, and then run the make_element command. The arguments to the make_element are: the name of the subplugin.

For example, the following commands create the test subplugin based on the plugin template and put the output files in the ./filter_plugins directory(new files:xst_filter_test.h and xst_filter_test.cc): ./make_element --name test

### 2. Fill in some functions

Add specific operations as needed in the invoke function or other functions.

### 3. Compile shared library(.so)

Add new subplugin in the ./filter_plugins/CMakeLists.txt, for example, for test subplugin: add_library(test_subplugin SHARED xst_filter_test.cc). Then run the ./filter_plugins/run.sh to complete compilation.

Shared library will be created in the ./filter_plugins/build/ directory, such as libtest_subplugin.so.

### 4. Use custom subplugin in the AdaFlow pipeline

Provide almighty filter to use custom subplugins in the AdaFlow pipeline, just write the shared library path of the custom subplugin and then it will work.

## Flow_c_extension Example

```
gst-launch-1.0 filesrc location=test04.mp4 ! decodebin ! videoconvert ! videoscale ! 
video/x-raw,width=640,height=480,format=RGB !  flow_tensor_convert  !  
flow_c_extension framework=libyolodetect_subplugin.so ! 
flow_tensor_decode ! videoconvert !  x264enc ! mp4mux ! filesink location=test04_subplugin.mp4
```




















