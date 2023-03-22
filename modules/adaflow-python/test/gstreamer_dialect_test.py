import os
import unittest
from adaflow.av.pipeline.dialects.gstreamer_pipeline import GStreamerPipeline
from adaflow.av.pipeline.model.struct import Struct
from jsonschema import ValidationError

Pipelines = {
    "p1": {
        "name": "test1",
        "backend": "Gstreamer",
        "schema_version": "1.0",
        "dialect": [
            "filesrc location={{parameters.input_location}}",
            "decodebin",
            "videoconvert",
            "videoscale",
            "video/x-raw,width=50,height=50,format=RGB",
            "videoconvert",
            "x264enc",
            "mp4mux",
            "filesink location={{parameters.output_location}}"
        ],
        "parameters_schema": {
            "type": "object",
            "properties": {
                "input_location": {
                    "type": "string",
                    "minLength": 5
                },
                "output_location": {
                    "type": "string",
                    "minLength": 5
                }
            }
        }
    },
    "p2": {
        "name": "test1",
        "backend": "Gstreamer",
        "dialect": "filesrc location=test04.mp4 ! decodebin ! videoconvert ! videoscale ! video/x-raw,width=50,height=50,format=RGB ! tensorconvert ! tfliteinfer config=../model/superresolution/lite-model_esrgan-tf2_1.tflite ! tensortransform way=clamp option=0:255 ! frameconvert ! videoconvert ! x264enc ! mp4mux ! filesink location=test04_super.mp4",
        "parameters_schema": {
            "type": "object",
            "properties": {}
        }
    },
    "p3": {
        "name": "test1",
        "backend": "Gstreamer",
        "dialect": [
            "filesrc location={{parameters.input_location}}",
            "decodebin",
            "videoconvert",
            "videoscale",
            "video/x-raw,width=50,height=50,format=RGB",
            "videoconvert",
            "x264enc",
            "mp4mux",
            "filesink location={{parameters.output_location}}"
        ],
        "parameters_schema": {
            "type": "object",
            "properties": {
                "input_location": {
                    "type": "string",
                    "default": "input.mp4"
                },
                "output_location": {
                    "type": "string",
                    "default": "output.mp4"
                },
                "test_path": {
                    "type": "string",
                    "default": "{{envs.PATH}}"
                }
            }
        }
    },
    "p4": {
        "name": "p4",
        "backend": "GStreamer",
        "dialect": [
            "{{F.source('src1')}}",
            "videoscale",
            "videoconvert",
            "video/x-raw,width={{parameters.width}},height={{parameters.height}},format=RGB",
            "x264enc",
            "mp4mux",
            "{{F.sink('sink1')}}"
        ],
        "parameters_schema": {
            "type": "object",
            "properties": {
                "width": {
                    "type": "integer",
                    "default": 800
                },
                "height": {
                    "type": "integer",
                    "default": 600
                }
            }
        }
    }
}

Tasks = {
    "t1": {"sinks": [], "sources": [], "parameters": {}},
    "t2": {
        "sinks": [],
        "sources": [],
        "parameters": {
            "input_location": "",
            "output_location": ""
        }
    },
    "t3": {
        "sources": [{"name": "src1", "type": "file", "location": "file.mp4"}],
        "sinks": [{"name": "sink1", "type": "gst", "element": "filesink", "properties": {"location": "output.mp4"}}]
    },
    "t4": {
        "sources": [{"name": "src1", "type": "uri", "uri": "https://example.com/input.mp4"}],
        "sinks": [{"name": "sink1", "type": "file", "location": "output.mp4"}]
    },
    "t5": {
        "sources": [{"name": "src1", "type": "application"}],
        "sinks": [{"name": "sink1", "type": "multifile", "location": "output.mp4"}]
    },
    "t6": {
        "sources": [{"name": "src1", "type": "file", "location": "file.mp4"}],
        "sinks": [{"name": "sink1", "type": "application"}]
    },
    "t7": {
        "sources": [{"name": "src1", "type": "file", "location": "file.mp4"}],
        "sinks": [{"name": "sink1", "type": "metadata", "format": "json", "location": "file:///usr/local"}]
    },
    "t8": {
        "sources": [{"name": "src1", "type": "file", "location": "file.mp4"}],
        "sinks": [{"name": "sink1", "type": "oss_bucket", "location": "oss://my-bucket/path/file.mp4"}]
    },

}


class GStreamerDialectTest(unittest.TestCase):

    def test_simple_template(self):
        pipeline = GStreamerPipeline(Pipelines["p2"], Tasks["t1"])
        self.assertEqual(pipeline.command, "filesrc location=test04.mp4 ! decodebin ! videoconvert ! videoscale ! "
                                           "video/x-raw,width=50,height=50,format=RGB ! tensorconvert ! tfliteinfer "
                                           "config=../model/superresolution/lite-model_esrgan-tf2_1.tflite ! "
                                           "tensortransform way=clamp option=0:255 ! frameconvert ! videoconvert ! "
                                           "x264enc ! mp4mux ! filesink location=test04_super.mp4")
        self.assertEqual(pipeline.parameters, {})

    def test_parameter_validation(self):
        with self.assertRaises(ValidationError) as context:
            pipeline = GStreamerPipeline(Pipelines["p1"], Tasks["t2"])
            pipeline.startup()
        self.assertTrue("too short" in str(context.exception))

    def test_parameter_default_values(self):
        pipeline = GStreamerPipeline(Pipelines["p3"], Tasks["t2"])
        self.assertEqual(pipeline.parameters["input_location"], "input.mp4")
        self.assertEqual(pipeline.parameters["output_location"], "output.mp4")
        self.assertEqual(pipeline.parameters["test_path"], os.environ["PATH"])

    def test_adaptive_source_and_sink(self):
        p1 = GStreamerPipeline(Pipelines["p4"], Tasks["t3"])
        self.assertEqual(p1.command, "filesrc name=src1 location=file.mp4 ! decodebin ! videoscale ! videoconvert ! "
                                     "video/x-raw,width=800,height=600,format=RGB ! x264enc ! mp4mux ! filesink "
                                     "location=output.mp4")

        p2 = GStreamerPipeline(Pipelines["p4"], Tasks["t4"])
        self.assertEqual(p2.command, "urisourcebin name=src1 uri=https://example.com/input.mp4 ! decodebin "
                                     "! videoscale ! videoconvert ! video/x-raw,width=800,height=600,"
                                     "format=RGB ! x264enc ! mp4mux ! filesink name=sink1 "
                                     "location=output.mp4")

        p3 = GStreamerPipeline(Pipelines["p4"], Tasks["t5"])
        self.assertEqual(p3.command, "appsrc name=src1 ! videoscale ! videoconvert ! video/x-raw,width=800,"
                                     "height=600,format=RGB ! x264enc ! mp4mux ! multifilesink name=sink1 "
                                     "location=output.mp4")

        p4 = GStreamerPipeline(Pipelines["p4"], Tasks["t6"])
        self.assertEqual(p4.command,
                         "filesrc name=src1 location=file.mp4 ! decodebin ! videoscale ! videoconvert ! video/x-raw,"
                         "width=800,height=600,format=RGB ! x264enc ! mp4mux ! appsink name=sink1")

        p5 = GStreamerPipeline(Pipelines["p4"], Tasks["t7"])
        self.assertEqual(p5.command,
                         "filesrc name=src1 location=file.mp4 ! decodebin ! videoscale ! videoconvert ! video/x-raw,"
                         "width=800,height=600,format=RGB ! x264enc ! mp4mux ! flow_metadata_sink name=sink1 "
                         "location=file:///usr/local format=json")

        p6 = GStreamerPipeline(Pipelines["p4"], Tasks["t8"])
        self.assertEqual(p6.command,
                         "filesrc name=src1 location=file.mp4 ! decodebin ! videoscale ! videoconvert ! video/x-raw,"
                         "width=800,height=600,format=RGB ! x264enc ! mp4mux ! flow_oss_upload_sink name=sink1 "
                         "location=oss://my-bucket/path/file.mp4")


if __name__ == '__main__':
    unittest.main()
