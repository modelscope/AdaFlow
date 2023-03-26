# Writing a Python Extension

## Overview

`flow_python_extension` element provides a callback to execute user-defined Python functions on every frame in the 
adaflow pipeline, and can be used for metadata conversion, inference post-processing, and other tasks.  
You can support a new metadata-process or a new post-processing after AI models inference by writing a python extension.

## Quick guide on writing a python extension
You can start writing a python extension easily by using code-template. The following is how to 
start writing a python extension with the template for `visualize the results` after 
ModelScope [realtime detector](https://modelscope.cn/models/damo/cv_cspnet_image-object-detection_yolox/summary). 
In this example, the target python extension name is `real_detector_vis.py`.

### fill a python function

```bash
from adaflow.av.data.av_data_packet import AVDataPacket
```
import adaflow api `AVDataPacket`, this class represents video frames - working with metadata which
belong to this video frame (image). Metadata describes inference results on VideoFrame level.
VideoFrame also provides access to underlying GstBuffer and GstVideoInfo describing frame's video information (such
as image width, height, channels, strides, etc.).

```bash
import cv2
...
```
import libraries for additional dependencies of your own functions.

```bash
class RealDetectorPost:
    def postprocess(self, frames: AVDataPacket, kwargs):
```
define Python class name and Python function name, and the function parameters are the frames(class AVDataPacket) and
kwargs(user-defined parameters which parsed by element flow_python_extension).

```bash
self.color = kwargs['color']
```
the color value of the resulting box and the content of yaml file(real_detector.yaml) which defined by user.
```bash
color:
  [255, 0, 0]
```

```bash
for frame in frames:
```
per frame process.

```bash
self.meta_data = frame.get_json_meta('detection')
self.image = frame.data()
scores = self.meta_data['scores']
boxes = self.meta_data['boxes']
labels = self.meta_data['labels']
```
get inference results and frame information, and the meta name `detection` is set by user in the previous plugin
`flow_modelscope_pipeline` as key `meta-key=detection`.

```bash
for idx in range(len(scores)):
    x1, y1, x2, y2 = boxes[idx]
    score = str(scores[idx])
    label = str(labels[idx])
    cv2.rectangle(self.image, (int(x1), int(y1)), (int(x2), int(y2)), self.color, 2)
    cv2.putText(self.image, label, (int(x1), int(y1) - 10),
    cv2.FONT_HERSHEY_PLAIN, 1, self.color)
    cv2.putText(self.image, score, (int(x1), int(y2) + 10),
    cv2.FONT_HERSHEY_PLAIN, 1, self.color)
```
user-defined functions：inference results show on the image, draw rectangles and put labels.

> **NOTE**: source code in the modules/adaflow-python/test/detection_repo/extension/real_detector_vis.py

### use port flow_python_extension
run your python extension by using element `flow_python_extension` as port, the specific calls are as follows:
```bash
... ! flow_python_extension input=real_detector.yaml module=real_detector_vis.py class= RealDetectorPost function = postprocess ! ...
```
> **NOTE**: source code in the  modules/adaflow-python/test/detection_repo/pipelines/real_detector/pipeline.json , and flow_python_extension supports features see in [flow_python_extension](built_in_elements.md)



