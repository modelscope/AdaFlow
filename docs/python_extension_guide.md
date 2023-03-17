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
import cv2

class RealDetectorPost:
    def postprocess(self, frames: AVDataPacket, kwargs):

        self.color = kwargs['color']

        for frame in frames:
            self.meta_data = frame.get_json_meta('detection')
            self.image = frame.data()
            scores = self.meta_data['scores']
            boxes = self.meta_data['boxes']
            labels = self.meta_data['labels']

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
TO DO :具体解释


#### use port flow_python_extension

### image pre-process


