# Copyright (c) Alibaba, Inc. and its affiliates.
"""
postprocess detection based on TensorRT
"""

from adaflow.av.data.av_data_packet import AVDataPacket
import cv2
import random
import numpy as np
from numpy import ndarray
from typing import List, Tuple, Union
random.seed(0)

# detection model classes
CLASSES = ('person', 'bicycle', 'car', 'motorcycle', 'airplane', 'bus',
           'train', 'truck', 'boat', 'traffic light', 'fire hydrant',
           'stop sign', 'parking meter', 'bench', 'bird', 'cat', 'dog',
           'horse', 'sheep', 'cow', 'elephant', 'bear', 'zebra', 'giraffe',
           'backpack', 'umbrella', 'handbag', 'tie', 'suitcase', 'frisbee',
           'skis', 'snowboard', 'sports ball', 'kite', 'baseball bat',
           'baseball glove', 'skateboard', 'surfboard', 'tennis racket',
           'bottle', 'wine glass', 'cup', 'fork', 'knife', 'spoon', 'bowl',
           'banana', 'apple', 'sandwich', 'orange', 'broccoli', 'carrot',
           'hot dog', 'pizza', 'donut', 'cake', 'chair', 'couch',
           'potted plant', 'bed', 'dining table', 'toilet', 'tv', 'laptop',
           'mouse', 'remote', 'keyboard', 'cell phone', 'microwave', 'oven',
           'toaster', 'sink', 'refrigerator', 'book', 'clock', 'vase',
           'scissors', 'teddy bear', 'hair drier', 'toothbrush')

# colors for per classes
COLORS = {
    cls: [random.randint(0, 255) for _ in range(3)]
    for i, cls in enumerate(CLASSES)
}

class ObjDetPost:
    def __init__(self):
        self.image = None
        self.meta_data = None
        self.color = None

    def postprocess(self, frames: AVDataPacket, kwargs):

        for frame in frames:
            self.meta_data = frame.get_json_meta('det')

            num_dets = self.meta_data['num_dets']
            nums = num_dets[0][0]

            scores = self.meta_data['scores'][0][:nums]
            bboxes = self.meta_data['bboxes'][:nums]
            labels = self.meta_data['labels'][0][:nums]

            self.image = frame.data()

            self.ori_height = frame.height
            self.ori_width = frame.width

            self.mod_height = 640
            self.mod_width =640

            rato_h = self.ori_height/self.mod_height
            rato_w = self.ori_width/self.mod_width

            for idx in range(nums):
                x1, y1, x2, y2 = bboxes[idx]
                x1 = x1*rato_w
                x2 = x2*rato_w
                y1 = y1*rato_h
                y2 = y2*rato_h

                score = scores[idx]
                cls_id = labels[idx]
                cls = CLASSES[cls_id]
                color = COLORS[cls]
                cv2.rectangle(self.image, (int(x1), int(y1)), (int(x2), int(y2)), color, 2)
                cv2.putText(self.image, f'{cls}:{score:.3f}', (int(x1), int(y1) - 10),
                    cv2.FONT_HERSHEY_PLAIN, 1, color)
