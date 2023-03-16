import unittest
from adaflow.av.pipeline.dialects.gstreamer_pipeline import GStreamerPipeline
from adaflow.av.pipeline.model.struct import Struct


class GStreamerDialectTest(unittest.TestCase):

    def test_simple_template(self):
        p = Struct.from_dict({
            "name": "test1",
            "backend": "Gstreamer",
            "dialect": "filesrc location=test04.mp4 ! decodebin ! videoconvert ! videoscale ! video/x-raw,width=50,height=50,format=RGB ! tensorconvert ! tfliteinfer config=../model/superresolution/lite-model_esrgan-tf2_1.tflite ! tensortransform way=clamp option=0:255 ! frameconvert ! videoconvert ! x264enc ! mp4mux ! filesink location=test04_super.mp4",
            "parameters": {
                "type": "object",
                "properties": {}
            }
        })
        t = Struct.from_dict({
            "sinks": [],
            "sources": [],
            "parameters": {}
        })
        pipeline = GStreamerPipeline(p, t)
        print(pipeline.command)


if __name__ == '__main__':
    unittest.main()
