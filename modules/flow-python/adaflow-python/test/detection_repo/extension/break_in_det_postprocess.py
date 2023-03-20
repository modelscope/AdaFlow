"""
break in detection post process
"""
from adaflow.av.data.av_data_packet import AVDataPacket
import numpy as np
import cv2
import os
import json
from adaflow.av.utils import NumpyArrayEncoder

class BreakInDetPostprocess:
    def __init__(self):
        pass
    def postprocess(self, frames: AVDataPacket, kwargs):
        """
        Post-processing the detection results.
        :param frames:construct AVDataPacket instance
        :param kwargs:parameters of user-defined functions
        :return:
        """
        ##user parameter
        self.outyaml = 'mass_smoke_det_res.yaml'
        self.output_path = kwargs['output_path']
        self.vis_flag = kwargs['vis_flag']
        self.is_video = kwargs['deploy']['rules']['is_video']
        self.region_polygon = kwargs['deploy']['rules']['region_polygon']
        self.frame_rate = kwargs['deploy']['rules']['frame_rate']
        self.det_thres = kwargs['deploy']['rules']['det_thres']

        ##frame by frame
        idx = 0
        for frame in frames:
            ## interval processing
            if idx % self.frame_rate ==0:
                self.meta_data = frame.get_json_meta('modelout')
                self.image = frame.data()
                scores = self.meta_data['scores']
                self.meta_data['alarms'] = []
                #private-postprocess
                for i, score in enumerate(scores):
                    if score >= self.det_thres:
                        box = self.meta_data['boxes'][i]
                        if self._check_box_status(box):
                            self.meta_data['alarms'].append(True)
                        else:
                            self.meta_data['alarms'].append(False)

                self.meta_data['alarms'] = np.array(self.meta_data['alarms'])

                if (self.vis_flag):
                    self._visualize(self.meta_data, self.image)

                ##write_result_json
                res = {
                      0: self.meta_data
                }

                self._write_result_yaml(self.output_path, res, self.frame_rate)

                ##add new meta with key
                frame.add_json_meta(self.meta_data, 'BreakInDetPost')

            idx+=1

            return True

    # ----------------------------private-------------------------------

    def _check_box_status(self, box) -> bool:
        """
        check if bbox center in region polygon.
        :param box: box (list): [x1, y1, x2, y2]
        :return:True: inside region polygon. False: not inside.
        """
        pts = np.array(self.region_polygon, np.int32)
        pts = pts.reshape((-1, 1, 2))
        cx = (box[2] + box[0]) / 2
        cy = (box[3] + box[1]) / 2
        center = (cx, cy)
        ret = cv2.pointPolygonTest(pts, center, False)
        if ret >= 0:
            return True
        return False

    def _visualize(self, detection_meta, image):
        labels = detection_meta['labels']
        bboxes = detection_meta['boxes']
        scores = detection_meta['scores']
        alarms = detection_meta['alarms']
        # draw det results
        for i, box in enumerate(bboxes):
            x1, y1, x2, y2 = box
            color = self.get_color(i)
            cv2.rectangle(image, (int(x1), int(y1)),
                          (int(x2), int(y2)), color, 2)
            cv2.putText(image,
                        f'{labels[i]}_{scores[i]:<.2f}_{alarms[i]}',
                        (int(x1), int(y1) - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.75, color, 2)
        return True

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

    def get_color(self, idx):
        idx = idx * 3
        color = ((37 * idx) % 255, (17 * idx) % 255, (29 * idx) % 255)

        return color