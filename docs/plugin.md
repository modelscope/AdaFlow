# Built-in plugins {#plugin}


| **plugin**                         | **description**                                                                                            |    
|:-----------------------------------|:-----------------------------------------------------------------------------------------------------------|
| flow_modelscope_pipeline           | run modelscope pipeline and produce result data. [Properties](#flow_modelscope_pipeline)                                  | 
| flow_python_extension              | provides a callback to execute user-defined python functions on every frame. [Properties](#flow_python_extension)      |  
| flow_metadata_aggregate            | aggregates inference results from multiple pipeline branches. [Properties](#flow_metadata_aggregate)                |   
| flow_video_aggregate               | manipulate merge multiple frames in a batch, and optionally produce metadata. [Properties](#frameconvertP) |   
| flow_metadata_sink                 | publishes the JSON metadata to files. [Properties](#flow_metadata_sink)                                      |  



## plugin properties

### flow_modelscope_pipeline 
<a id="hrgbflipP"></a>
- lum-only: only apply filter on luminance. Default: false
- Example: image horizontal flip for video frame only on luminance

```bash
... ! hrgbflip lum-only = true ! ...
```


### flow_python_extension
<a id="videoaceP"></a>
- filtersize: the size of the median filter. Default: 5
- lum-only: only apply filter on luminance
- Example: image median filter with size 5 for video frame only on luminance

```bash
... ! videoace filtersize = 5 lum-only = true ! ...
```


### flow_metadata_aggregate
<a id="tensorconvertP"></a>
- Example: convert video frame to tensor
  
```bash
... ! tensorconvert ! ...
```


### flow_metadata_sink
<a id="frameconvertP"></a>
- Example: convert tensor to video frame

```bash
... ! tensorconvert ! ... ! frameconvert ! ...
```



