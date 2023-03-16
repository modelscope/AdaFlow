# Built-in plugins {#plugin}


| **plugin**                         |                                                                               |                                          | IOS |                                          Mac                              | Windows | **description**                                                           |
|:-----------------------------------|:------------------------------------------------------------------------------|:----------------------------------------:|:---:|:-------------------------------------------------------------------------:|:-------:|:--------------------------------------------------------------------------|
| flow_modelscope_pipeline           |                                                                               |                                          | N/A |                                      |   N/A   | image horizontal flip for video frame. [Properties](#hrgbflipP)           |
| flow_python_extension              |                                                                               |                                          | N/A |  |   N/A   | image madian filter for video frame. [Properties](#videoaceP)             |
| flow_metadata_aggregate            |                                                                               |                                          | N/A |  |   N/A   | convert video frame to tensor. [Properties](#tensorconvertP)              |
| flow_video_aggregate               |                                                                               |                                          | N/A |  |   N/A   | convert tensor to video frame. [Properties](#frameconvertP)               |
| flow_metadata_sink                 |                                                                               |                                          | N/A |  |   N/A   | tensor transform: add, mul, div, clamp, linear, zscore. [Properties](#tensortransformP) |
| mnninfer                           |                                                                               |                   N/A                    | N/A |  |   N/A   | inference for tensor by MNN deep learning framework backends.  [Properties](#mnninferP)   |
| tfliteinfer                        |                                                                               |                                          | N/A | |   N/A   | inference for tensor by Tflite deep learning framework backends. [Properties](#tfliteinferP)|
| xnninfer                           |                                                                               |                                          | N/A |  |   N/A   | a plugin temple for tensor filter.    |
| tensorresize                       |                                                                               |                                          | N/A |  |   N/A   | tensor bilinear resize. [Properties](#tensorresizeP)  |
| xcompositor                        |                                                                               |                                          | N/A |  |   N/A   | network post-processing：such as segment task. [Properties](#xcompositorP)  |
| trtinfer                           |                                                                               |                   N/A                    | N/A |                                          N/A                              |   N/A   | inference for tensor by TensorRT deep learning framework backends. [Properties](#trtinferP) |
| tensoraggregator                   |                                                                               |                   N/A                    | N/A |                                          N/A                              |   N/A   | aggregate tensor stream. [Properties](#tensoraggregatorP) |




## plugin properties

### hrgbflip 
<a id="hrgbflipP"></a>
- lum-only: only apply filter on luminance. Default: false
- Example: image horizontal flip for video frame only on luminance

```bash
... ! hrgbflip lum-only = true ! ...
```


### videoace
<a id="videoaceP"></a>
- filtersize: the size of the median filter. Default: 5
- lum-only: only apply filter on luminance
- Example: image median filter with size 5 for video frame only on luminance

```bash
... ! videoace filtersize = 5 lum-only = true ! ...
```


### tensorconvert
<a id="tensorconvertP"></a>
- Example: convert video frame to tensor
  
```bash
... ! tensorconvert ! ...
```


### frameconvert
<a id="frameconvertP"></a>
- Example: convert tensor to video frame

```bash
... ! tensorconvert ! ... ! frameconvert ! ...
```


### tensortransform
<a id="tensortransformP"></a>
- way: way used for transforming tensor. Default: -1, "unknown"
- (0): arithmetic
    - A way for arithmetic operations with tensor
    - An option should be provided as option=add|mul|div:NUMBER...
    - Example 1: Element-wise add 25 and multiply 4

    ```bash
    ... ! tensorconvert ! tensortransform way=arithmetic option=add:25,mul:4 ! ...
    ```

    - Example 2: Element-wise divide 25 and subtract 25

    ```bash
    ... ! tensorconvert ! tensortransform way=arithmetic option=div:25,add:-25 ! ...
    ```

- (1): clamp
    - Way for clamping all elements of tensor into the range
    - An option should be provided as option=CLAMP_MIN:CLAMP_MAX
    - Example: clamp element-wise of tensor into the range of [0, 255]
  
    ```bash
    ... ! tensorconvert ! tensortransform way=clamp option=0:255 ! ...
    ```

- (2): stand
    - A way for statistical standardization or normalization of tensor
    - An option should be provided as option=linear|zscore:false|true, where `linear|zscore` for statistical standardization and `false|true` whether to deal with per-channel
    - Example: linear normalization of tensor with per-channel
  
    ```bash
    ... ! tensorconvert ! tensortransform way=stand option=linear:true ! ...
    ```

- (3): dimchg
    - A way for changing tensor dimensions
    - An option should be provided as option=nchw|nhwc
    - Example: Move 1st dim to 2nd dim (i.e., [n][H][W][C] ==> [n][C][H][W])

    ```bash
    ... ! tensorconvert ! tensortransform way=dimchg option=nchw ! ...
    ```


### mnninfer
<a id="mnninferP"></a>
- gpu-id: GPU to use for inference. Default: 0
- config: path to a mnn model file. Default: "xxx"
- flexible: if shape of model is flexible
- lum-only: only apply filter on luminance when the format of video is YUV. Default:true
- use-gpu : infer use GPU(opencl). Default:false
- Example 1: inference for tensor by MNN deep learning framework backends

```bash
... ! mnninfer config =model.mnn ! ...
```

- Example 2: inference for tensor by MNN deep learning framework backends using GPU

```bash
... ! mnninfer config =model.mnn use-gpu=true ! ...
```


### tfliteinfer
<a id="tfliteinferP"></a>
- gpu-id: GPU to use for inference. Default: 0
- config: path to a tflite model file. Default: "xxx"
- lum-only: only apply filter on luminance when the format of video is YUV. Default:true
- Example: inference for tensor by Tflite deep learning framework backends

```bash
... ! tfliteinfer config =model.tflite ! ...
```


### tensorresize
<a id="tensorresizeP"></a>
- width: the output resized width
- height: the output resized height
- Example: tensor bilinear resize with width-1280 and height-720

```bash
... ! tensorconvert ! tensorresize width=1280 height=720 ! ...
```  


### xcompositor
<a id="xcompositorP"></a>
- operator: blending operator to use for blending this pad over the previous ones. Default: segment
- bgcolor: segment background color
- Example: blend two streams for segment task

```bash
... ! tee name=mytee ! queue ! ... ! xcompositor name=mix bgcolor=red ! ... !  mix. 
```  


### trtinfer
<a id="trtinferP"></a>
- gpu-id: GPU to use for inference. Default: 0
- onnx: path to a onnx model file
- trt: path to save trt model file
- lum-only: only apply filter on luminance when the format of video is YUV. Default:true
- flexible: if shape of model is flexible. Default: false
- input-name: name of model input layer. Default: input
- sr: size change ratio. Default: 1.0
- Example 1: inference for tensor by TensorRT deep learning framework backends

```bash
... ! trtinfer onnx=model.onnx trt=model.trt ! ...
```

- Example 2: inference for tensor by TensorRT deep learning framework backends with flexible shape SR 2.0

```bash
... ! trtinfer onnx=model.onnx trt=model.trt flexible=true input-name=input0 sr=2 ! ...
```


### tensoraggregator
<a id="tensoraggregatorP"></a>
- frames-in: the number of frames in incoming buffer. Default: 1
- frames-out: the number of frames in outgoing buffer. Default: 1
- frames-flush: the number of frames to flush (0 to flush all output). Default: 0
- Example 1: 1 frame in, 5 frames out, flush 2 frames

```bash
... ! tensoraggregator frames-in=1 frames-out=5 frames-flush=2 ! ...
```

- Example 2: 5 frame in, 1 frames out, flush 0 frames

```bash
... ! tensoraggregator frames-in=5 frames-out=1 frames-flush=0 ! ...
```