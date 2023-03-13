from typing import Dict
import gi
import networkx as nx
from adaflow.av.pipeline import PipelineComposer
from .base_pipeline import BasePipeline
from ..model.pipeline import Pipeline
from ..model.task import Task

# gi.require_version("Gst", "1.0")
# gi.require_version("GstApp", "1.0")
# gi.require_version("GstVideo", "1.0")
# from gi.repository import Gst, GLib, GObject, GstApp, GstVideo


class GStreamerPipeline(BasePipeline):
    def __init__(self, pipeline: Pipeline) -> None:
        super().__init__(pipeline)

    def startup(self, task: Task):
        pass

    def stop(self):
        pass

    def shutdown(self):
        pass

    def push(self, src_name: str, data_packet):
        pass

    def pop(self, sink_name: str) -> AVDataPacket:
        pass
