"""
smoke detection post process
"""
from adaflow.utils.video_frame import AVDataPacket
import numpy as np
import cv2
import copy
import operator
import os
import json
from adaflow.utils import NumpyArrayEncoder

class SmokeDetPostprocess:
    def __init__(self):
        pass
    def postprocess(self, frames: AVDataPacket, kwargs):
        ##user parameter
        self.outyaml = 'mass_smoke_det_res.yaml'
        self.output_path = kwargs['output_path']
        self.vis_flag = kwargs['vis_flag']
        metric_type = kwargs['deploy']['rules']['type']
        self.is_video = kwargs['deploy']['rules']['is_video']
        self.frame_rate = kwargs['deploy']['rules']['frame_rate']
        self.metric_func = METRIC_FUNCS[metric_type]
        self.metric_thred = kwargs['deploy']['rules']['threshold']
        self.metric_com_func = operator.le if metric_type == 'dist' else operator.ge

        ##frame by frame
        idx = 0
        for frame in frames:
            ## interval processing
            if idx % self.frame_rate ==0:
                human_det_res = frame.get_json_meta('human')
                cigs_det_res = frame.get_json_meta('cigare')
                self.image = frame.data()
                smoke_det_res = self._smoke_det(human_det_res, cigs_det_res)

                res = {
                    0:
                        dict(human_res=human_det_res,
                             cigs_res=cigs_det_res,
                             smoke_res=smoke_det_res),  # frame index with 0
                    }

                self._write_result_yaml(self.output_path, res, self.frame_rate)
                if (self.vis_flag):
                    self._visualize(self.image, res)

                ##add new meta with key
                frame.add_json_meta(smoke_det_res, 'SmokeDetPost')

            idx+=1

            return True

    # ----------------------------private-------------------------------

    def _smoke_det(self, human_det_dict, cigs_det_dict, re_order=True):

        human_det_dict = {'scores': np.array(human_det_dict['scores']),
                          'boxes': np.array(human_det_dict['boxes']),
                          'labels': np.array(human_det_dict['labels'])}

        cigs_det_dict = {'scores': np.array(cigs_det_dict['scores']),
                         'boxes': np.array(cigs_det_dict['boxes']),
                         'labels': np.array(cigs_det_dict['labels'])}

        smoke_det_dict = copy.deepcopy(human_det_dict)
        smoke_det_dict['smoke'] = []

        human_num = len(human_det_dict.get('scores', []))
        cigs_num = len(cigs_det_dict.get('scores', []))
        # boundary cases (i.e., no human or no cigs)
        if human_num == 0 or cigs_num == 0:
            smoke_det_dict['smoke'] = [
                dict(flag=False, cigs=None) for _ in range(human_num)
            ]
            return smoke_det_dict

        # sort in descending order
        if re_order:
            human_order = human_det_dict['scores'].argsort()[::-1]
            cigs_order = cigs_det_dict['scores'].argsort()[::-1]
            human_det_dict['scores'] = human_det_dict['scores'][human_order]
            human_det_dict['labels'] = [
                human_det_dict['labels'][i] for i in human_order
            ]
            human_det_dict['boxes'] = human_det_dict['boxes'][human_order]
            cigs_det_dict['scores'] = cigs_det_dict['scores'][cigs_order]
            cigs_det_dict['labels'] = [
                cigs_det_dict['labels'][i] for i in cigs_order
            ]
            cigs_det_dict['boxes'] = cigs_det_dict['boxes'][cigs_order]

        metric_mat = self.metric_func(cigs_det_dict['boxes'],
                                      human_det_dict['boxes'])
        human_assign = [[False, None] for _ in range(human_num)]

        for i in range(cigs_num):
            max_id = np.argmax(metric_mat[i, :])
            flag = self.metric_com_func(metric_mat[i, max_id],
                                        self.metric_thred)
            if flag and (not human_assign[max_id][0]):
                human_assign[max_id][0] = True
                human_assign[max_id][1] = dict(
                    flag=True,
                    cigs=dict(
                        score=cigs_det_dict['scores'][i],
                        box=cigs_det_dict['boxes'][i]))

        for i in range(human_num):
            if human_assign[i][0]:
                smoke_det_dict['smoke'].append(human_assign[i][1])
            else:
                smoke_det_dict['smoke'].append(dict(flag=False, cigs=None))

        return smoke_det_dict


    def _write_result_yaml(self, output_path, res, interval):
        json_path = os.path.join(output_path, self.outyaml)

        if (os.path.exists(json_path)):
            with open(json_path, 'r') as f:
                json_data = json.load(f)
                num_data = len(json_data)

            with open(json_path, 'w') as f:
                pix_num = num_data * interval
                json_data[pix_num] = res[0]
                json.dump(json_data, f, indent=1, cls=NumpyArrayEncoder)
                f.write('\n')
        else:
            with open(json_path, 'w') as f:
                json.dump(res, f, indent=1, cls=NumpyArrayEncoder)
                f.write('\n')

        return True

    def _visualize(self, im, res):
        for (score, label, bbox) in zip(res[0]['cigs_res']['scores'],
                                        res[0]['cigs_res']['labels'],
                                        res[0]['cigs_res']['boxes']):
            x1, y1, x2, y2 = bbox
            cv2.rectangle(im, (int(x1), int(y1)), (int(x2), int(y2)),
                          (255, 0, 0), 1)  # blue bbox for cigs
            im = cv2.putText(im, f'{label}:{score:.2f}',
                             (int(x1), int(y1)), cv2.FONT_HERSHEY_SIMPLEX,
                             1.2, (255, 0, 0), 1)

        for (score, label, bbox, smoke) in zip(res[0]['smoke_res']['scores'],
                                               res[0]['smoke_res']['labels'],
                                               res[0]['smoke_res']['boxes'],
                                               res[0]['smoke_res']['smoke']):
            x1, y1, x2, y2 = bbox
            if smoke['flag']:
                cv2.rectangle(im, (int(x1), int(y1)), (int(x2), int(y2)),
                              (0, 0, 255), 1)  # red bbox for smoking person
                im = cv2.putText(im, f'{label}:{score:.2f}',
                                 (int(x1), int(y1)),
                                 cv2.FONT_HERSHEY_SIMPLEX, 1.2,
                                 (0, 0, 255), 1)
            else:
                cv2.rectangle(im, (int(x1), int(y1)), (int(x2), int(y2)),
                              (0, 255, 0),
                              1)  # green bbox for no smoking person
                im = cv2.putText(im, f'{label}:{score:.2f}',
                                 (int(x1), int(y1)),
                                 cv2.FONT_HERSHEY_SIMPLEX, 1.2,
                                 (0, 255, 0), 1)
        total_person_num = len(res[0]['smoke_res']['smoke'])
        smoke_person_num = sum(x['flag']
                               for x in res[0]['smoke_res']['smoke'])
        im = cv2.putText(
            im,
            f'Smoke number/Total number: {smoke_person_num}/{total_person_num}',
            (20, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 1)

        return True


def _calculate_iou(array_a, array_b):
    """calculate the iou of two array

    Args:
        array_a (np.array): (N, 4), the 1st dim is number of bbox, the 2nd dim is the coordinate of (x1, y1, x2, y2)
        array_b (np.array): (M, 4), the 1st dim is number of bbox, the 2nd dim is the coordinate of (x1, y1, x2, y2)

    Returns:
        np.array: (N, M), the iou matrix of array_a and array_b
    """

    N = len(array_a)
    M = len(array_b)
    assert N > 0 and M > 0

    a_mat = array_a[:, np.newaxis, :]
    b_mat = array_b[np.newaxis, ...]
    # extend a and b to a matrix in the same shape
    a_mat = np.tile(a_mat, (1, M, 1))
    b_mat = np.tile(b_mat, (N, 1, 1))

    # calculate iou
    a_w = a_mat[..., 2] - a_mat[..., 0] + 1
    a_h = a_mat[..., 3] - a_mat[..., 1] + 1
    b_w = b_mat[..., 2] - b_mat[..., 0] + 1
    b_h = b_mat[..., 3] - b_mat[..., 1] + 1
    a_mat_areas = a_w * a_h
    b_mat_areas = b_w * b_h
    xx1 = np.maximum(a_mat[..., 0], b_mat[..., 0])
    yy1 = np.maximum(a_mat[..., 1], b_mat[..., 1])
    xx2 = np.minimum(a_mat[..., 2], b_mat[..., 2])
    yy2 = np.minimum(a_mat[..., 3], b_mat[..., 3])
    inter_w = np.maximum(0.0, xx2 - xx1 + 1)
    inter_h = np.maximum(0.0, yy2 - yy1 + 1)
    inter_areas = inter_w * inter_h

    iou = inter_areas / (a_mat_areas + b_mat_areas - inter_areas)

    return iou


def _calculate_ioa(array_a, array_b):
    """calculate the ioa of two array

    Args:
        array_a (np.array): (N, 4), the 1st dim is number of bbox, the 2nd dim is the coordinate of (x1, y1, x2, y2)
        array_b (np.array): (M, 4), the 1st dim is number of bbox, the 2nd dim is the coordinate of (x1, y1, x2, y2)

    Returns:
        np.array: (N, M), the ioa matrix of array_a and array_b
    """

    N = len(array_a)
    M = len(array_b)
    assert N > 0 and M > 0

    a_mat = array_a[:, np.newaxis, :]
    b_mat = array_b[np.newaxis, ...]
    # extend a and b to a matrix in the same shape
    a_mat = np.tile(a_mat, (1, M, 1))
    b_mat = np.tile(b_mat, (N, 1, 1))

    # calculate ioa
    a_w = a_mat[..., 2] - a_mat[..., 0] + 1
    a_h = a_mat[..., 3] - a_mat[..., 1] + 1
    a_mat_areas = a_w * a_h
    # b_mat_areas = b_w * b_h
    xx1 = np.maximum(a_mat[..., 0], b_mat[..., 0])
    yy1 = np.maximum(a_mat[..., 1], b_mat[..., 1])
    xx2 = np.minimum(a_mat[..., 2], b_mat[..., 2])
    yy2 = np.minimum(a_mat[..., 3], b_mat[..., 3])
    inter_w = np.maximum(0.0, xx2 - xx1 + 1)
    inter_h = np.maximum(0.0, yy2 - yy1 + 1)
    inter_areas = inter_w * inter_h

    # ioa = inter_areas / (a_mat_areas + b_mat_areas - inter_areas)
    ioa = inter_areas / a_mat_areas

    return ioa


def _calculate_dist(array_a, array_b):
    """calculate the dist of two array

    Args:
        array_a (np.array): (N, 4), the 1st dim is number of bbox, the 2nd dim is the coordinate of (x1, y1, x2, y2)
        array_b (np.array): (M, 4), the 1st dim is number of bbox, the 2nd dim is the coordinate of (x1, y1, x2, y2)

    Returns:
        np.array: (N, M), the dist matrix of array_a and array_b
    """

    N = len(array_a)
    M = len(array_b)
    assert N > 0 and M > 0

    a_mat = array_a[:, np.newaxis, :]
    b_mat = array_b[np.newaxis, ...]
    # extend a and b to a matrix in the same shape
    a_mat = np.tile(a_mat, (1, M, 1))
    b_mat = np.tile(b_mat, (N, 1, 1))
    a_c_mat = np.zeros((N, M, 2))
    b_c_mat = np.zeros((N, M, 2))

    # calculate dist
    # center point
    a_c_mat[..., 0] = (a_mat[..., 0] + a_mat[..., 2]) / 2
    a_c_mat[..., 1] = (a_mat[..., 1] + a_mat[..., 3]) / 2
    b_c_mat[..., 0] = (b_mat[..., 0] + b_mat[..., 2]) / 2
    b_c_mat[..., 1] = (b_mat[..., 1] + b_mat[..., 3]) / 2
    diff_c_mat = a_c_mat - b_c_mat

    dist = np.linalg.norm(diff_c_mat, axis=-1)

    return dist


METRIC_FUNCS = dict(
    dist=_calculate_dist,
    ioa=_calculate_ioa,
    iou=_calculate_iou,
)