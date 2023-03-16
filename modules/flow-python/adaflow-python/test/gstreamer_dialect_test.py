import unittest
from adaflow.av.pipeline.model.pipeline import Pipeline
from adaflow.av.pipeline.model.task import Task
from adaflow.av.pipeline.dialects.gstreamer_pipeline import GStreamerPipeline

class GStreamerDialectTest(unittest.TestCase):
    def test_simple_template(self):
        p = Pipeline.from_dict({
            "name": "test1",
            "backend": "Gstreamer",
            "dialect": "filesrc location=test04.mp4 ! decodebin ! videoconvert ! videoscale ! video/x-raw,width=50,height=50,format=RGB ! tensorconvert ! tfliteinfer config=../model/superresolution/lite-model_esrgan-tf2_1.tflite ! tensortransform way=clamp option=0:255 ! frameconvert ! videoconvert ! x264enc ! mp4mux ! filesink location=test04_super.mp4"
        })
        t = Task()
        pipeline = GStreamerPipeline(p, t)
        print(pipeline.command)


if __name__ == '__main__':
    unittest.main()
