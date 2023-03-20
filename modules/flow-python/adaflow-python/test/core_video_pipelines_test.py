import logging
import time
import unittest
from adaflow.av.pipeline.pipeline_factory import PipelineFactory
from adaflow.av.pipeline.dialects.gst_context import GstContext
from pathlib import Path

logging.basicConfig(level=logging.DEBUG)


class CoreVideoPipelinesTest(unittest.TestCase):
    factory = PipelineFactory.create(Path(__file__).parent.joinpath("./core_video_repo"))

    def test_mp4_mux(self):
        builder = self.factory.pipeline("mp4_mux").source({
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


    def test_readable_pipeline(self):
        cnt = 30 * 5
        builder = self.factory.readable_pipeline("mp4_mux").source({
            "name": "src1",
            "type": "gst",
            "element": "videotestsrc",
            "filter": "video/x-raw,width=800,height=600,format=RGB,framerate=30/1",
            "properties": {
                "num-buffers": cnt
            }
        }).sink({
            "name": "sink1",
            "type": "application",
            "properties": {
                "emit-signals": "true",
                "sync": "false"
            }
        })
        with GstContext():
            with builder.build() as pipeline:
                while not pipeline.is_done:
                    av_packet = pipeline.pop()
                    if av_packet is not None:
                        logging.info("av_packet frame %s", len(av_packet))
                    else:
                        logging.info("None is popped")

                self.assertEqual(pipeline.total_buffers_count, cnt)




if __name__ == '__main__':
    unittest.main()
