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

    @property
    def emitter(self):
        return self._emitter

    @property
    def pipeline(self):
        return self._pipeline

    def set_pipeline(self, pipeline: PipelineComposer):
        self._pipeline = pipeline
        return self

    def set_emitter(self, emitter: EventEmitter) -> PipelineFactoryType:
        self._emitter = emitter
        return self

    def build_pipeline(self):
        if self._pipeline.gstreamer_backend():
            from .dialects.gstreamer_pipeline import GStreamerPipeline
            return GStreamerPipeline(pipeline_composer=self._pipeline)
        raise RuntimeError("unsupported pipeline backend: %s" % self._pipeline.gstreamer_backend())











