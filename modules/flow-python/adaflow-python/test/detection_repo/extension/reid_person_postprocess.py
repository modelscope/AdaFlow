"""
reid person in detection post process
"""
from adaflow.av.utils.video_frame import AVDataPacket
import numpy as np
import logging
logging.basicConfig(level=logging.DEBUG)

class ReidPersonPostprocess:
    def __init__(self):
        pass
    def postprocess(self, frames: AVDataPacket, kwargs):
        """
        Post-processing the reid person results.
        :param frames:construct AVDataPacket instance
        :param kwargs:parameters of user-defined functions
        :return
        """
        ##frame by frame
        for frame in frames:
            feat_1 = frame.get_json_meta('model1')['img_embedding'][0]
            feat_2 = frame.get_json_meta('model2')['img_embedding'][0]
            feat_1 = np.array(feat_1)
            feat_2 = np.array(feat_2)
            feat_norm_1 = feat_1 / np.linalg.norm(feat_1)
            feat_norm_2 = feat_2 / np.linalg.norm(feat_2)
            score = np.dot(feat_norm_1, feat_norm_2)
            logging.info(f'cosine score is: {score}')

            return True
