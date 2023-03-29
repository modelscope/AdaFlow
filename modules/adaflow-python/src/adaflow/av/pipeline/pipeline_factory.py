# Copyright (c) Alibaba, Inc. and its affiliates.
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
    """
    The uniform interface to build a pipeline. The `PipelineFactory` is often binded to a pipeline repository folder
    structure for quick access to vairous resources and builder API.

    Current supported pipeline types are:
    * GStreamerPipeline - a basic Gstreamer-backed pipeline that has lifecycle control.
    * ReadalbeGStreamerPipeline - a GStreamer-backed pipeline that have readable access to sink data
    * WrtiableGStreamerPipeline - a GStreamer-backed pipeline that have writable access to source data
    * DuplexGstreamerPipeline - a GStreamer-backed pipeline that have both read and write access to internal data
    """

    @staticmethod
    def create(repository_path: pathlib.Path) -> PipelineFactoryType:
        """Static method to create a factory instance for a repository.

        Args:
            repository_path: the path object pointing to a repository folder

        Returns:
            PipelineFactory instance

        """
        return PipelineFactory(repository_path)

    def __init__(self, repository_path: pathlib.Path) -> None:
        """Default constructor

        Args:
            repository_path: the path object pointing to a repository folder
        """
        super().__init__()
        self._path = repository_path
        self._log = logging.Logger("PipelineFactory")

    @property
    def log(self) -> logging.Logger:
        """
        Returns:
            a `logging.Logger` object of this class

        """
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
        """Builder factory method for a `ReadableGStreamerPipelineBuilder`

        Args:
            pipeline_id: pipeline name

        Returns:
            builder object for the pipeline
        """
        p = self._load_pipeline_dsl(pipeline_id)
        return ReadableGStreamerPipelineBuilder().pipeline(p)

    def writable_pipeline(self, pipeline_id: str) -> WritableGstreamerPipelineBuilder:
        """Builder factory method for a `WritableGstreamerPipeline`

        Args:
            pipeline_id: pipeline name

        Returns:
            builder object for the pipeline
        """
        p = self._load_pipeline_dsl(pipeline_id)
        return WritableGstreamerPipelineBuilder().pipeline(p)

    def duplex_pipeline(self, pipeline_id: str) -> DuplexGstreamerPipelineBuilder:
        """Builder factory method for a `DuplexGstreamerPipelineBuilder`

        Args:
            pipeline_id: pipeline name

        Returns:
            builder object for the pipeline

        """
        p = self._load_pipeline_dsl(pipeline_id)
        return DuplexGstreamerPipelineBuilder().pipeline(p)

    def pipeline(self, pipeline_id: str) -> GStreamerPipelineBuilder:
        """Builder factory method for a `GStreamerPipelineBuilder`

        Args:
            pipeline_id: pipeline name

        Returns:
            builder object for the pipeline
        """
        p = self._load_pipeline_dsl(pipeline_id)
        return GStreamerPipelineBuilder().pipeline(p)

