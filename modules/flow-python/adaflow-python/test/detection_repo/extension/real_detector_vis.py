"""
visualization after realtime detector
"""
from adaflow.av.utils.video_frame import AVDataPacket
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


