# Copyright (c) Alibaba, Inc. and its affiliates.
import logging
import time
import unittest

from adaflow.av.pipeline.pipeline_factory import PipelineFactory
from adaflow.av.pipeline.dialects.gst_context import GstContext
from pathlib import Path

logging.basicConfig(level=logging.DEBUG)


class DetectionPipelinesTest(unittest.TestCase):

    factory = PipelineFactory.create(Path(__file__).parent.joinpath("./detection_repo"))

    def test_break_in(self):
        builder = self.factory.pipeline("break_in").source({
            "name": "src1",
            "type": "file",
            "location": "./detection_repo/resource/data/test_walker1.jpeg"
        }).sink({
            "name": "sink1",
            "type": "file",
            "location": "./detection_repo/resource/data/break_walk_res.jpg"
        })
        with GstContext():
            with builder.build() as pipeline:
                logging.info(pipeline.command)
                while not pipeline.is_done:
                    time.sleep(1)

    def test_mot_counting(self):
        builder = self.factory.pipeline("mot_counting").source({
            "name": "src1",
            "type": "file",
            "location": "./detection_repo/resource/data/test_walker1.jpeg"
        }).sink({
            "name": "sink1",
            "type": "file",
            "location": "./detection_repo/resource/data/maas_test_detection_ok.jpg"
        })
        with GstContext():
            with builder.build() as pipeline:
                logging.info(pipeline.command)
                while not pipeline.is_done:
                    time.sleep(1)

    def test_reid_person(self):
        builder = self.factory.pipeline("reid_person").source({
            "name": "src1",
            "type": "file",
            "location": "./detection_repo/resource/data/image_reid_person.jpg"
        }).sink({
            "name": "sink1",
            "type": "fakesink"
        }).source({
            "name": "src2",
            "type": "file",
            "location": "./detection_repo/resource/data/image_reid_person.jpg"
        })
        with GstContext():
            with builder.build() as pipeline:
                logging.info(pipeline.command)
                while not pipeline.is_done:
                    time.sleep(1)

    def test_smoke_det(self):
        builder = self.factory.pipeline("smoke_det").source({
            "name": "src1",
            "type": "file",
            "location": "./detection_repo/resource/data/smoke_a388.jpg"
        }).sink({
            "name": "sink1",
            "type": "file",
            "location": "./detection_repo/resource/data/smoke_det_res.jpg"
        })
        with GstContext():
            with builder.build() as pipeline:
                logging.info(pipeline.command)
                while not pipeline.is_done:
                    time.sleep(1)

    def test_real_detector(self):
        builder = self.factory.pipeline("real_detector").source({
            "name": "src1",
            "type": "file",
            "location": "./detection_repo/resource/data/test_walker1.jpeg"
        }).sink({
            "name": "sink1",
            "type": "file",
            "location": "./detection_repo/resource/data/maas_test_detection_vis.jpg"
        })
        with GstContext():
            with builder.build() as pipeline:
                logging.info(pipeline.command)
                while not pipeline.is_done:
                    time.sleep(1)


if __name__ == '__main__':
    unittest.main()
