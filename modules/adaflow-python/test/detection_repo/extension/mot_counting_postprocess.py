# Copyright (c) Alibaba, Inc. and its affiliates.
"""
    mot counting model postprocess
"""
from adaflow.av.data.av_data_packet import AVDataPacket
import cv2
import logging


class MotCountingPostprocess:

    def __init__(self):
        self.in_flag = None
        self.line = None
        self.coord = None
        self.horizontal = None
        self.is_video = None
        self.vis_flag = None
        self.input_path = None
        self.output_path = None

    def postprocess(self, frames: AVDataPacket, kwargs):
        """
        Post-processing the detection results.
        :param frames:construct AVDataPacket instance
        :param kwargs:parameters of user-defined functions
        :return
        """
        self.output_path = kwargs['output_path']
        self.input_path = kwargs['input_path']
        self.vis_flag = kwargs['vis_flag']
        self.is_video = kwargs['deploy']['rules']['is_video']
        self.horizontal = kwargs['deploy']['rules']['horizontal']
        self.coord = kwargs['deploy']['rules']['coord']
        self.in_flag = kwargs['deploy']['rules']['in_flag']
        self.line = kwargs['deploy']['rules']
        # frame by frame
        for frame in frames:
            mot_res = frame.get_json_meta('mot')
            res = dict()
            res['mot_res'] = mot_res
            res['final_res'] = self._count(mot_res)
            if self.vis_flag:
                self._visualize(self.input_path, res, self.output_path)

            return True

    def _check_box_status(self, box, line) -> bool:
        """
        check whether input box is inside the line
        :param box:[x1, y1, x2, y2]
        :param line:the boundary line, which consists of line direction and coordinate.
                    in_flag: indicates the definition of the line.
                    True means the box is inside when box center < coord.
                    False means the box is inside when box center > coord.
        :return:box_status: True means inside the line while False means outside.
        """

        x2, y2 = box[-2:]
        coord = line['coord']
        if line['horizontal']:
            if line['in_flag']:
                return y2 < coord
            else:
                return y2 > coord
        else:
            if line['in_flag']:
                return x2 < coord
            else:
                return x2 > coord

    def _count(self, mot_bboxes) -> dict:
        in_id_list = []
        out_id_list = []
        in_num = 0
        out_num = 0
        output_res = []
        if mot_bboxes is None or len(mot_bboxes['boxes']) == 0:
            logging.info('No tracklets found!')
            return output_res

        for cur_bboxes, ids in zip(mot_bboxes['boxes'], mot_bboxes['labels']):
            for idx in range(len(cur_bboxes)):
                cur_bbox = cur_bboxes[idx]
                cur_status = self._check_box_status(cur_bbox, self.line)
                id = ids[idx]
                if (id not in in_id_list) and (id not in out_id_list):
                    if cur_status:
                        in_id_list.append(id)
                    else:
                        out_id_list.append(id)
                    continue
                else:
                    if cur_status:
                        if id in out_id_list:
                            out_id_list.remove(id)
                            in_id_list.append(id)
                            in_num += 1
                    else:
                        if id in in_id_list:
                            in_id_list.remove(id)
                            out_id_list.append(id)
                            out_num += 1
            output_res.append((in_num, out_num))

        return output_res

    def _visualize(self, input_video, res, save_path) -> None:
        cap = cv2.VideoCapture(input_video)
        frame_idx = 0
        while True:
            ret, frame = cap.read()
            if frame is None:
                break
            if frame_idx == 0:
                size = (frame.shape[1], frame.shape[0])
                fourcc = cv2.VideoWriter_fourcc('M', 'J', 'P', 'G')
                video_writer = cv2.VideoWriter(save_path, fourcc,
                                               cap.get(cv2.CAP_PROP_FPS), size,
                                               True)

            # draw line
            if self.line['horizontal']:
                cv2.line(frame, (0, int(self.line['coord'])),
                         (frame.shape[1], int(self.line['coord'])),
                         (0, 0, 255), 2)
            else:
                cv2.line(frame, (int(self.line['coord']), 0),
                         (int(self.line['coord']), frame.shape[0]),
                         (0, 0, 255), 2)

            # draw counting_res
            counting_text = 'in: {}, out: {}'.format(
                res['final_res'][frame_idx][0], res['final_res'][frame_idx][1])
            cv2.putText(frame, counting_text, (30, 100),
                        cv2.FONT_HERSHEY_PLAIN, 2, (0, 255, 0), 2)

            # draw box
            bboxes = res['mot_res']['boxes'][frame_idx]
            ids = res['mot_res']['labels'][frame_idx]
            for box, obj_id in zip(bboxes, ids):
                x1, y1, x2, y2 = box[0:]
                id_text = '{}'.format(int(obj_id))
                color = self._get_color(obj_id)
                cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)),
                              color, 2)
                cv2.putText(frame, id_text, (int(x1), int(y1) + 30),
                            cv2.FONT_HERSHEY_PLAIN, 1, (0, 255, 0), 2)
            video_writer.write(frame)
            frame_idx += 1
        video_writer.release
        cap.release()
        return save_path

    def _get_color(self, idx):
        idx = idx * 3
        color = ((37 * idx) % 255, (17 * idx) % 255, (29 * idx) % 255)

        return color
