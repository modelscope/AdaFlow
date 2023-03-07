from typing import Dict

import pyee
from pyee import EventEmitter
from .pipeline_composer import PipelineComposer

class PipelineRunner:

    def __init__(self, pipeline: PipelineComposer, emitter: EventEmitter = None) -> None:
        super().__init__()
        if not emitter:
            self._emitter = pyee.EventEmitter()
        self._pipeline = pipeline

    @property
    def emitter(self):
        return self._emitter

    def execute(self, parameters: Dict[str, any]):
        dialect = None
        if self._pipeline.gstreamer_backend():
            from .dialects.gstreamer_pipeline_dialect import GStreamerPipelineDialect
            dialect = GStreamerPipelineDialect()
        dialect.translate()
        pass








