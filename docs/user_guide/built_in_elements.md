# Built-in elements
Elements are the basic building blocks of pipelines. Elements to perform a specific operation on the incoming frame and
the resulting frame is then pushed downstream for further processing. Each element will get data from its upstream element,
process it and then output the data for the next element to process.

In this tutorial we provide documentation for the use of two categories of elements, ```AdaFlow elements``` and ```GStreamer handy elements```.

# AdaFlow elements

| **Element**                                                                   | **Description**                                                              |    
|:------------------------------------------------------------------------------|:-----------------------------------------------------------------------------|
| [flow_modelscope_pipeline](#flow_modelscope_pipeline)                         | run modelscope pipeline and produce result data.                             | 
| [flow_python_extension](#flow_python_extension)                               | provides a callback to execute user-defined python functions on every frame. |  
| [flow_metadata_aggregate](#flow_metadata_aggregate)                           | aggregates inference results from multiple pipeline branches.                |   
| [flow_video_aggregate](#flow_video_aggregate)                                 | aggregate or dis-aggregate the video frame.                                  |   
| [flow_metadata_sink](#flow_metadata_sink)                                     | publishes the JSON metadata to files.                                        |  


## <a id="flow_modelscope_pipeline">flow_modelscope_pipeline</a>
### Supported features

flow_modelscope_pipeline is a plugin to run modelscope pipeline and produce result data.

This plugin pulls and infers models from [ModelScope](https://modelscope.cn/home), 
and attaches results as new GstFLOWJSONMeta instance to passed buffer.

flow_modelscope_pipeline gets the model task with ```task```, 
gets the model id with ```id``` and attaches results to the GstBuffer with keyword ```meta-key```.


### Properties
- task: modelscope model task, support list of task types (string)
- id: modlescope modle id
- add-meta: whether to attach metadata to the GstBuffer(Default, True)
- meta-key: the keyword of the results which attach to the GstBuffer
- input: image or video input path

### Usage Examples
- sample1: [passvitb-image-reid-person](https://modelscope.cn/models/damo/cv_passvitb_image-reid-person_market/summary)

```bash
... ! flow_modelscope_pipeline task=image-reid-person id = damo/cv_passvitb_image-reid-person_market meta-key=modelout ! ...
```

- sample2: low level CV tasks, such as super resolution, color enhance, image cartoon which are pixel process and can 
write results to buffer pixel, set ```add-meta=False``` 

```bash
... ! flow_modelscope_pipeline task=image-super-resolution id = damo/cv_rrdb_image-super-resolution add-meta=False ! ...
```

## <a id="flow_python_extension">flow_python_extension</a>

### Supported features
flow_python_extension is a plugin to provide a callback to execute user-defined Python functions on every frame,
and can be used for metadata conversion, inference post-processing, and other tasks.

### Properties

- input: parameters of user-defined functions file path(such as yaml file)
- module: Python module name, usually a python file path
- class: (optional) Python class name
- function: Python function name(Default, postprocess)

### Usage Examples
- sample: break in detection postprocess
```bash
... ! flow_python_extension input=../resource/config/break_in_count_deploy.yaml module=break_in_det_postprocess.py class= BreakInDetPostprocess function = postprocess ! ...
```

## <a id="flow_metadata_aggregate">flow_metadata_aggregate</a>

### Supported features
flow_metadata_aggregate is a plugin to aggregate inference results from multiple pipeline branches.
The new aggregation result will be attached to the pass buffer. 

### Properties
- frame-num: frame number in the pass GstBuffer.(Default 1)

### Usage Examples
- sample: passvitb-image-reid-person
```bash
gst-launch-1.0 filesrc location=../resource/data/image_reid_person.jpg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
flow_modelscope_pipeline task=image-reid-person id = damo/cv_passvitb_image-reid-person_market meta-key=model1 ! \
flow_metadata_aggregate name = mixer ! flow_python_extension module=reid_person_postprocess.py class= ReidPersonPostprocess ! fakesink \
filesrc location=../resource/data/image_reid_person.jpg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
flow_modelscope_pipeline task=image-reid-person id = damo/cv_passvitb_image-reid-person_market meta-key=model2 ! mixer.
```

## <a id="flow_video_aggregate">flow_video_aggregate</a>

### Supported features
flow_video_aggregate is a plugin to aggregate or dis-aggregate the video frame using GstAdapter.

This plugin handles the buffer with the unit **frame**.
Each incoming or outgoing buffer which may contain one or multi frames.

flow_video_aggregate gets the size of one frame with ```frames-in```, aggregates the frames, and pushes a buffer with ```frames-out``` frames.
After pushing an outgoing buffer, flow_video_aggregate flushes the ```frames-flush``` frames.

With larger ```frames-in``` values and smaller ```frames-out``` values, the output stream may have more frames than its input stream: ```dis-aggregation```.

For visualization, flow_video_aggregate with the properties ```frames-in=3```, ```frames-out=5```, ```frames-flush=2```

```
Incoming buffer
----------------------------------------------
|  1st buffer  |  2nd buffer  |  3rd buffer  |  
----------------------------------------------
| 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 
----------------------------------------------
Outgoing buffer
----------------------------------------------
|    1st out-buffer      |
----------------------------------------------
 flushed  |      2nd out-buffer   |
----------------------------------------------
            flushed |       3rd out-buffer   |
----------------------------------------------
 
```

### Properties
- frames-in: the number of frames in incoming buffer. (Default 1)
- frames-out: the number of frames in outgoing buffer. (Default 1)
- frames-flush: the number of frames to flush. (Default 0)

### Usage Examples
- sample: process step 5 frames in the video detection
```bash
... ! flow_video_aggregate frames-in=1  frames-out=5 ! ...
```

## <a id="flow_metadata_sink">flow_metadata_sink</a>

### Supported features
flow_metadata_sink is a plugin to publish the JSON metadata to files.
mqtt/kafka will to be supported in the future.

### Properties
- method: publish method.(Default: file)
- filepath:absolute path to output file for publishing inferences
- fileformat:structure(one line or multi-lines) of JSON objects in the file.(Default：json)

### Usage Examples
- sample: save results to metasink_json.ymal using json-lines format
```bash
... ! flow_metadata_sink filepath = metasink_json.ymal fileformat=json-lines ! ...
```

# GStreamer handy elements

This tutorial gives a list of handy GStreamer elements that are worth knowing. More elements can be found [gstreamer plugins](https://gstreamer.freedesktop.org/documentation/plugins_doc.html?gi-language=c).

| **Element**                   | **Description**                                                                                        |    
|:------------------------------|:-------------------------------------------------------------------------------------------------------|
| [filesrc](#filesrc)           | reads data from a file in the local file system.                                                       | 
| [urisourcebin](#urisourcebin) | an element for accessing URIs in a uniform manner.                                                     |  
| [decodebin](#decodebin)       | auto-magically constructs a decoding pipeline using available decoders and demuxers via auto-plugging. |   
| [videoscale](#videoscale)     | resizes video frames.                                                                                  |   
| [video/x-raw](#video/x-raw)   | specify explicitly what type of data flows between elements in a gstreamer pipeline.                   |  
 | [tee](#tee)                   | splits data to multiple pads.                                                                          |
| [queue](#queue)               | creates new threads of execution for some parts of the pipeline.                                       |  
| [jpegenc](#jpegenc)           | encodes jpeg images.                                                                                   |
| [x264enc](#x264enc)           | encodes raw video into H264 compressed data.                                                           |
| [mp4mux](#mp4mux)             | merges streams (audio and video) into ISO MPEG-4 (.mp4) files.                                                           |
| [filesink](#filesink)         | writes incoming data to a file in the local file system.                                               |
| [fakesink](#fakesink)         | simply swallows any data fed to it.                                                                    |


## <a id="filesrc">filesrc</a>

### Supported features
This element reads a local file and produces media with ANY Caps.

### Properties
- location: location of the file to read.

### Usage Examples

```bash
... ! filesrc location=song.mp4 ! ...
```

## <a id="urisourcebin">urisourcebin</a>

### Supported features
This element decodes data from a URI into raw media. It selects a source element that can handle the given URI scheme 
and connects it to a decodebin element

### Properties
- uri: URI to decode.

### Usage Examples

```bash
... ! uridecodebin uri=https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm ! ...
```


## <a id="decodebin">decodebin</a>

### Supported features
This element automatically constructs a decoding pipeline using available decoders and demuxers via auto-plugging until 
raw media is obtained. It is used internally by uridecodebin which is often more convenient to use, as it creates 
a suitable source element as well. It replaces the old decodebin element. It acts like a demuxer,
so it offers as many source pads as streams are found in the media

### Usage Examples

```bash
... ! decodebin ! ...
```

## <a id="videoscale">videoscale</a>

### Supported features
This element resizes video frames. This element supports a wide range of color spaces including various YUV and RGB formats 
and is therefore generally able to operate anywhere in a pipeline.

### Usage Examples

```bash
... ! videoscale ! ...
```

## <a id="video/x-raw">video/x-raw</a>

### Supported features
Raw Video Media Types. Specify explicitly what type of data flows between elements in a gstreamer pipeline.

### Properties
- width: the width of the image in pixels.
- height: the height of the image in pixels.
- framerate: the framerate of the video, 0/1 for variable framerate.
- format: the format of the video.
### Usage Examples
usually used with videoscale element

```bash
... ! videoscale ! video/x-raw,width=720,height=360,format=RGB ! ...
```

## <a id="tee">tee</a>

### Supported features
Split data to multiple pads. Branching the data flow is useful when e.g. capturing a video where the video is shown 
on the screen and also encoded and written to a file.

One needs to use separate queue elements (or a multiqueue) in each branch to provide separate threads for each branch. 
Otherwise a blocked dataflow in one branch would stall the other branches.

### Properties
- name: the name of the split data.

### Usage Examples
usually used with queue element

```bash
gst-launch-1.0 filesrc location=song.ogg ! decodebin ! tee name=t ! queue ! audioconvert ! audioresample ! autoaudiosink t. ! queue ! audioconvert ! goom ! videoconvert ! autovideosink
```

## <a id="queue">queue</a>

### Supported features
Basically, a queue performs two tasks:  
- Data is queued until a selected limit is reached. Any attempt to push more buffers into the queue blocks the pushing 
thread until more space becomes available.  
- The queue creates a new thread on the source Pad to decouple the processing on sink and source Pads.

### Usage Examples
usually used with tee element

```bash
gst-launch-1.0 filesrc location=song.ogg ! decodebin ! tee name=t ! queue ! audioconvert ! audioresample ! autoaudiosink t. ! queue ! audioconvert ! goom ! videoconvert ! autovideosink
```

## <a id="jpegenc">jpegenc</a>

### Supported features
Encodes jpeg images.

### Usage Examples

```bash
... ! jpegenc ! ...
```

## <a id="x264enc">x264enc</a>

### Supported features
This element encodes raw video into H264 compressed data, also otherwise known as MPEG-4 AVC (Advanced Video Codec).

### Properties
- qp-max: maximum quantizer(Default value : 51)
- qp-min: minimum quantizer(Default value : 10)

### Usage Examples

```bash
... ! x264enc qp-max=20 ! ...
```


## <a id="mp4mux">mp4mux</a>

### Supported features
This element merges streams (audio and video) into ISO MPEG-4 (.mp4) files.

### Usage Examples

```bash
... ! x264enc qp-max=20 ! mp4mux ! filesink location=video.mp4 ! ...
```

## <a id="filesink">filesink</a>

### Supported features
Write incoming data to a file in the local file system.
### Properties
- location: location of the file to write.

### Usage Examples

```bash
... ! filesink location=capture1.jpeg ! ...
```

## <a id="fakesink">fakesink</a>

### Supported features
Dummy sink that swallows everything.

### Usage Examples

```bash
... ! fakesink ! ...
```