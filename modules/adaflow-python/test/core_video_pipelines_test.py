import logging
import threading
import time
import unittest

import numpy as np

from adaflow.av.pipeline.pipeline_factory import PipelineFactory
from adaflow.av.data.av_data_packet import AVDataPacket
from adaflow.av.pipeline.dialects.gst_context import GstContext
from adaflow.av.pipeline.dialects.duplex_gstreamer_pipeline import DuplexGstreamerPipeline
from pathlib import Path

import gi
gi.require_version("Gst", "1.0")
gi.require_version("GstApp", "1.0")
gi.require_version("GstVideo", "1.0")
from gi.repository import Gst, GLib, GObject, GstApp, GstVideo  # noqa:F401,F402

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
        builder = self.factory.readable_pipeline("adaptive_io_sample").source({
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
                        self.assertEqual(av_packet.width, 800)
                        self.assertEqual(av_packet.height, 600)
                        self.assertEqual(av_packet.channel, 3)
                        self.assertEqual(av_packet[0].data(flag=Gst.MapFlags.READ).shape, (600, 800, 3))
                        self.assertTrue(av_packet[0].get_json_meta("foo"))

                    else:
                        logging.info("None is popped")

                self.assertEqual(pipeline.total_buffers_count, cnt)

    def test_writable_pipeline(self):
        cnt = 30 * 5
        builder1 = self.factory.writable_pipeline("adaptive_io_sample").caps_filter(800, 600).source({
            "name": "src1",
            "type": "application"
        }).sink({
            "name": "sink1",
            "type": "gst",
            "element": "fakesink"
        })
        builder2 = self.factory.readable_pipeline("adaptive_io_sample").source({
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
            with builder1.build() as writable, builder2.build() as readable:
                while not readable.is_done:
                    av_packet = readable.pop()
                    if av_packet is not None:
                        self.assertEqual(av_packet.width, 800)
                        self.assertEqual(av_packet.height, 600)
                        self.assertEqual(av_packet.channel, 3)
                        writable.push(av_packet)

    def test_duplex_pipeline(self):
        width = 800
        height = 600
        channels = 3
        num_buffers = 150
        builder = self.factory.duplex_pipeline("adaptive_io_sample").source({
            "name": "src1",
            "type": "application"
        }).sink({
            "name": "sink1",
            "type": "application",
            "properties": {
                "emit-signals": "true",
                "sync": "false"
            }
        }).caps_filter(800, 600)

        frames = []

        def produce_frames(p: DuplexGstreamerPipeline) -> None:
            for _ in range(num_buffers):
                buffer = np.random.randint(low=0, high=255, size=(
                    height, width, channels), dtype=np.uint8)
                p.push(buffer)
            p.end()

        def consume_frames(p: DuplexGstreamerPipeline) -> [AVDataPacket]:
            while not p.is_done:
                data = p.pop()
                if data is not None:
                    frames.append(data)

        with GstContext():
            with builder.build() as p:
                consumer = threading.Thread(target=consume_frames, args=(p,))
                producer = threading.Thread(target=produce_frames, args=(p,))
                consumer.start()
                producer.start()
                producer.join()
                consumer.join()
                logging.info("found %s frames after join", len(frames))
                self.assertEqual(len(frames), num_buffers)


if __name__ == '__main__':
    unittest.main()
