# user-defined Python functions
## overview
`flow_python_extension` element provides a callback to execute user-defined Python functions on every frame in the 
gstreamer pipeline, and can be used for metadata conversion, inference post-processing, and other tasks. 
This page shows how to implement your own function coded in Python.

## samples
### detection post-process
Use AdaFlow pipeline run ModelScope [realtime detector](https://modelscope.cn/models/damo/cv_cspnet_image-object-detection_yolox/summary) 
and get detection results(such box,labels,scores), then want to `visualize the results`. Just need to write `a python function` 
and then use `flow_python_extension` as port can achieve.

#### write a python function

```bash
from adaflow.utils.video_frame import AVDataPacket
```
import adaflow api `AVDataPacket`, this class represents video frames - working with metadata which
belong to this video frame (image). Metadata describes inference results on VideoFrame level.
VideoFrame also provides access to underlying GstBuffer and GstVideoInfo describing frame's video information (such
as image width, height, channels, strides, etc.).

```bash
import cv2
...
```
import some user-defined process related 3rd libraries.

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
per frame peocess.

```bash
self.meta_data = frame.get_json_meta('detection')
self.image = frame.data()
scores = self.meta_data['scores']
boxes = self.meta_data['boxes']
labels = self.meta_data['labels']
```
get inference results and frame information.

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
user-defined functions：inference results show on the image.

> **NOTE**: source code in the modules/flow-python/adaflow-python/test/detection_repo/extension/real_detector_vis.py

#### use port flow_python_extension
```bash
... ! flow_python_extension input=real_detector.yaml module=real_detector_vis.py class= RealDetectorPost function = postprocess ! ...
```


