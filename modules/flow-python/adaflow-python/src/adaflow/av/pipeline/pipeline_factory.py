from typing import Dict, TypeVar

import pyee
from pyee import EventEmitter
from .pipeline_composer import PipelineComposer
from .dialects.gstreamer_pipeline import GStreamerPipeline
from .model.task import Task

PipelineFactoryType = TypeVar("PipelineFactory", bound="PipelineFactory")


class PipelineFactory:

    @staticmethod
    def create():
        return PipelineFactory()

    def json_path(self, json_path: str) -> PipelineFactoryType:
        self._json_path = json_path
        return self

    def emitter(self, emitter: EventEmitter) -> PipelineFactoryType:
        self._emitter = emitter
        return self

    def build_pipeline(self):
        # if self._pipeline.gstreamer_backend():
        #     from .dialects.gstreamer_pipeline import GStreamerPipeline
        #     return GStreamerPipeline(pipeline_composer=self._pipeline)
        # raise RuntimeError("unsupported pipeline backend: %s" % self._pipeline.gstreamer_backend())
        pass











