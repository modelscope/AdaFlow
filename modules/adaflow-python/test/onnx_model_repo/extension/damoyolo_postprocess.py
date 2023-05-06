# Copyright (c) Alibaba, Inc. and its affiliates.
"""
postprocess detection based on TensorRT
"""

from adaflow.av.data.av_data_packet import AVDataPacket
import cv2
import random
import numpy as np
from numpy import ndarray
import torch
from typing import List, Tuple, Union
import torchvision

def multiclass_nms(multi_bboxes,
                   multi_scores,
                   score_thr,
                   iou_thr,
                   max_num=100,
                   score_factors=None):
    """NMS for multi-class bboxes.

    Args:
        multi_bboxes (Tensor): shape (n, #class*4) or (n, 4)
        multi_scores (Tensor): shape (n, #class), where the last column
            contains scores of the background class, but this will be ignored.
        score_thr (float): bbox threshold, bboxes with scores lower than it
            will not be considered.
        nms_thr (float): NMS IoU threshold
        max_num (int): if there are more than max_num bboxes after NMS,
            only top max_num will be kept.
        score_factors (Tensor): The factors multiplied to scores before
            applying NMS

    Returns:
        tuple: (bboxes, labels), tensors of shape (k, 5) and (k, 1). Labels \
            are 0-based.
    """
    num_classes = multi_scores.size(1)
    # exclude background category
    if multi_bboxes.shape[1] > 4:
        bboxes = multi_bboxes.view(multi_scores.size(0), -1, 4)
    else:
        bboxes = multi_bboxes[:, None].expand(
            multi_scores.size(0), num_classes, 4)
    scores = multi_scores
    # filter out boxes with low scores
    valid_mask = scores > score_thr  # 1000 * 80 bool

    # We use masked_select for ONNX exporting purpose,
    # which is equivalent to bboxes = bboxes[valid_mask]
    # (TODO): as ONNX does not support repeat now,
    # we have to use this ugly code
    # bboxes -> 1000, 4
    bboxes = torch.masked_select(
        bboxes,
        torch.stack((valid_mask, valid_mask, valid_mask, valid_mask),
                    -1)).view(-1, 4)  # mask->  1000*80*4, 80000*4
    if score_factors is not None:
        scores = scores * score_factors[:, None]
    scores = torch.masked_select(scores, valid_mask)
    labels = valid_mask.nonzero(as_tuple=False)[:, 1]

    if bboxes.numel() == 0:
        bboxes = multi_bboxes.new_zeros((0, 5))
        labels = multi_bboxes.new_zeros((0, ), dtype=torch.long)
        scores = multi_bboxes.new_zeros((0, ))

        return bboxes, scores, labels

    keep = torchvision.ops.batched_nms(bboxes, scores, labels, iou_thr)

    if max_num > 0:
        keep = keep[:max_num]

    return bboxes[keep], scores[keep], labels[keep]


def postprocess_gfocal(prediction, num_classes, conf_thre=0.05, nms_thre=0.7):
    assert prediction.shape[0] == 1
    for i, image_pred in enumerate(prediction):
        # If none are remaining => process next image
        if not image_pred.size(0):
            continue
        multi_bboxes = image_pred[:, :4]
        multi_scores = image_pred[:, 4:]
        detections, scores, labels = multiclass_nms(multi_bboxes, multi_scores,
                                                    conf_thre, nms_thre, 500)

    return detections, scores, labels

class ObjDetPost:
    def __init__(self):
        self.image = None
        self.meta_data = None
        self.color = None

    def postprocess(self, frames: AVDataPacket, kwargs):

        for frame in frames:
            self.meta_data = frame.get_json_meta('det')

            preds = torch.Tensor(np.array(tuple(self.meta_data.values())))

            bboxes, scores, labels_idx = postprocess_gfocal(preds, 80, 0.6)
            bboxes = bboxes.cpu().numpy()
            scores = scores.cpu().numpy()
            labels_idx = labels_idx.cpu().numpy()

            self.image = frame.data()

            for (score, label, box) in zip(scores, labels_idx, bboxes):
                cv2.rectangle(self.image, (int(box[0]), int(box[1])),
                      (int(box[2]), int(box[3])), (0, 0, 255), 2)
                cv2.putText(self.image, f'{score:.2f}', (int(box[0]), int(box[1])), 1, 1.0, (0, 255, 0), thickness=1, lineType=8)

