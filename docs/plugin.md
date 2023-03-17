# Built-in plugins {#plugin}


| **plugin**                         | **description**                                                              |    
|:-----------------------------------|:-----------------------------------------------------------------------------|
| flow_modelscope_pipeline           | run modelscope pipeline and produce result data.                             | 
| flow_python_extension              | provides a callback to execute user-defined python functions on every frame. |  
| flow_metadata_aggregate            | aggregates inference results from multiple pipeline branches.                |   
| flow_video_aggregate               | aggregate or dis-aggregate the video frame.                                  |   
| flow_metadata_sink                 | publishes the JSON metadata to files.                                        |  



# plugins 

## flow_modelscope_pipeline

### Supported features

flow_modelscope_pipeline is a plugin to run modelscope pipeline and produce result data.

This plugin pulls and infers models from [ModelScope](https://modelscope.cn/home), 
and attaches results as new GstFLOWJSONMeta instance to passed buffer.

flow_modelscope_pipeline gets the model task with ```task```, 
gets the model id with ```id``` and attaches results to the GstBuffer with keyword ```meta-key```.


### Properties
- task: modelscope model task, support list of task types (string)
- id: modlescope modle id
- meta-key: the keyword of the results which attach to the GstBuffer
- input: image or video input path

### Usage Examples
- sample: [passvitb-image-reid-person](https://modelscope.cn/models/damo/cv_passvitb_image-reid-person_market/summary)

```bash
... ! flow_modelscope_pipeline task=image-reid-person id = damo/cv_passvitb_image-reid-person_market meta-key=modelout ! ...
```


## flow_python_extension

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


## flow_metadata_aggregate

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

## flow_video_aggregate
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

## flow_metadata_sink
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



