import logging
import pathlib
from typing import Dict, TypeVar

from .dialects.gstreamer_pipeline import GStreamerPipelineBuilder
from .dialects.readable_gstreamer_pipeline import ReadableGStreamerPipeline, ReadableGStreamerPipelineBuilder
from .dialects.writable_gstreamer_pipeline import WritableGstreamerPipeline, WritableGstreamerPipelineBuilder
from .dialects.duplex_gstreamer_pipeline import DuplexGstreamerPipeline, DuplexGstreamerPipelineBuilder
import json

PipelineFactoryType = TypeVar("PipelineFactoryType", bound="PipelineFactory")

PIPELINE_DSL_FILE_NAME = "pipeline.json"


class PipelineFactory:

    @staticmethod
    def create(repository_path: pathlib.Path) -> PipelineFactoryType:
        return PipelineFactory(repository_path)

    def __init__(self, repository_path: pathlib.Path) -> None:
        super().__init__()
        self._path = repository_path
        self._log = logging.Logger("PipelineFactory")

    @property
    def log(self) -> logging.Logger:
        return self._log

    def _load_pipeline_dsl(self, pipeline_name: str):
        json_filepath = self._path.joinpath("pipelines", pipeline_name, PIPELINE_DSL_FILE_NAME)
        self.log.info("search pipeline at %s" % str(json_filepath))
        if json_filepath.exists():
            with open(json_filepath) as j:
                j.seek(0)
                return json.load(j)
        else:
            raise RuntimeError("pipeline id %s doesn't exist" % pipeline_name)

    def readable_pipeline(self, pipeline_id: str) -> ReadableGStreamerPipelineBuilder:
        p = self._load_pipeline_dsl(pipeline_id)
        return ReadableGStreamerPipelineBuilder().pipeline(p)

    def writable_pipeline(self, pipeline_id: str) -> WritableGstreamerPipelineBuilder:
        p = self._load_pipeline_dsl(pipeline_id)
        return WritableGstreamerPipelineBuilder().pipeline(p)

    def duplex_pipeline(self, pipeline_id: str) -> DuplexGstreamerPipelineBuilder:
        p = self._load_pipeline_dsl(pipeline_id)
        return DuplexGstreamerPipelineBuilder().pipeline(p)

    def pipeline(self, pipeline_id: str) -> GStreamerPipelineBuilder:
        p = self._load_pipeline_dsl(pipeline_id)
        return GStreamerPipelineBuilder().pipeline(p)

