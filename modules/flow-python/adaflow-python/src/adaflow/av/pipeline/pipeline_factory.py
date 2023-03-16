import pathlib
from typing import Dict, TypeVar

import pyee
from pyee import EventEmitter
from .pipeline_composer import PipelineComposer
from .dialects.gstreamer_pipeline import GStreamerPipeline
from .model.task import Task
from .dialects.readable_gstreamer_pipeline import ReadableGStreamerPipeline, ReadableGStreamerPipelineBuilder
from .dialects.writable_gstreamer_pipeline import WritableGstreamerPipeline, WritableGstreamerPipelineBuilder
from .dialects.duplex_gstreamer_pipeline import DuplexGstreamerPipeline, DuplexGstreamerPipelineBuilder
from .model.pipeline import Pipeline
import json

PipelineFactoryType = TypeVar("PipelineFactory", bound="PipelineFactory")

PIPELINE_DSL_FILE_NAME = "pipeline.json"



class PipelineFactory:

    @staticmethod
    def create(repository_path: pathlib.Path) -> PipelineFactoryType:
        return PipelineFactory(repository_path)

    def __init__(self, repository_path: pathlib.Path) -> None:
        super().__init__()
        self._path = repository_path

    def _load_pipeline_dsl(self, id: str) -> Pipeline:
        json_filepath = self._path.joinpath(id, PIPELINE_DSL_FILE_NAME)
        if json_filepath.exists():
            with open(json_filepath) as j:
                return json.load(j, object_hook=lambda x: Pipeline(**x))
                j.seek(0)
        else:
            raise RuntimeError("pipeline id %s doesn't exist" % id)

    def readable_pipeline(self, pipeline_id: str) -> ReadableGStreamerPipelineBuilder:
        p = self._load_pipeline_dsl(pipeline_id)
        return ReadableGStreamerPipelineBuilder().pipeline(p)

    def writable_pipeline(self, pipeline_id: str) -> WritableGstreamerPipelineBuilder:
        p = self._load_pipeline_dsl(pipeline_id)
        return WritableGstreamerPipelineBuilder().pipeline(p)

    def duplex_pipeline(self, pipeline_id: str) -> DuplexGstreamerPipelineBuilder:
        p = self._load_pipeline_dsl(pipeline_id)
        return DuplexGstreamerPipelineBuilder().pipeline(p)



