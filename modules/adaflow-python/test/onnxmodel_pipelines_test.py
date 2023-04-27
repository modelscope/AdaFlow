# Copyright (c) Alibaba, Inc. and its affiliates.
import logging
import time
import unittest

from adaflow.av.pipeline.pipeline_factory import PipelineFactory
from adaflow.av.pipeline.dialects.gst_context import GstContext
from pathlib import Path
import torch

logging.basicConfig(level=logging.DEBUG)


class DetectionPipelinesTest(unittest.TestCase):

    repo_path = Path(__file__).parent.joinpath("./onnx_model_repo")

    factory = PipelineFactory.create(repo_path)

    @unittest.skipUnless(torch.cuda.is_available(), "skip test when cuda is unavailable")
    def test_onnx_sr(self):
        builder = self.factory.pipeline("SR").source({
            "name": "src1",
            "type": "file",
            "location": self.repo_path.joinpath("./resource/data/dogs.jpg")
        }).sink({
            "name": "sink1",
            "type": "file",
            "location": self.repo_path.joinpath("./resource/data/dogs_sr2x.jpg")
        })
        with GstContext():
            with builder.build() as pipeline:
                logging.info(pipeline.command)
                while not pipeline.is_done:
                    time.sleep(1)

    @unittest.skipUnless(torch.cuda.is_available(), "skip test when cuda is unavailable")
    def test_onnx_segment(self):
        builder = self.factory.pipeline("segment").source({
            "name": "src1",
            "type": "file",
            "location": self.repo_path.joinpath("./resource/data/segment.jpg")
        }).sink({
            "name": "sink1",
            "type": "file",
            "location": self.repo_path.joinpath("./resource/data/segment_res.jpg")
        })
        with GstContext():
            with builder.build() as pipeline:
                logging.info(pipeline.command)
                while not pipeline.is_done:
                    time.sleep(1)

    @unittest.skipUnless(torch.cuda.is_available(), "skip test when cuda is unavailable")
    def test_onnx_detection(self):
        builder = self.factory.pipeline("detection").source({
            "name": "src1",
            "type": "file",
            "location": self.repo_path.joinpath("./resource/data/zidane.jpg")
        }).sink({
            "name": "sink1",
            "type": "fakesink"
        }).source({
            "name": "src2",
            "type": "file",
            "location": self.repo_path.joinpath("./resource/data/zidane_res.jpg")
        })
        with GstContext():
            with builder.build() as pipeline:
                logging.info(pipeline.command)
                while not pipeline.is_done:
                    time.sleep(1)




if __name__ == '__main__':
    unittest.main()
