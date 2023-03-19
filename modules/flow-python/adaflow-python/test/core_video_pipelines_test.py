import logging
import time
import unittest
from adaflow.av.pipeline.pipeline_factory import PipelineFactory
from adaflow.av.pipeline.dialects.gst_context import GstContext
from pathlib import Path

logging.basicConfig(level=logging.DEBUG)


class CoreVideoPipelinesTest(unittest.TestCase):

    def test_mp4_mux(self):
        factory = PipelineFactory.create(Path(__file__).parent.joinpath("./core_video_repo"))
        builder = factory.pipeline("mp4_mux").source({
            "name": "src1",
            "type": "gst",
            "element": "videotestsrc",
            "filter": "video/x-raw,width=800,height=600,format=RGB,framerate=30/1",
            "properties": {
                "num-buffers": 30 * 5
            }
        }).sink({
            "name": "sink1",
            "type": "file",
            "location": "/tmp/output.mp4"
        })
        with GstContext():
            with builder.build() as pipeline:
                logging.info(pipeline.command)
                while not pipeline.is_done:
                    time.sleep(1)


if __name__ == '__main__':
    unittest.main()
