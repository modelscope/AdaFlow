from typing import Dict

from .struct import Struct


class SinkModel(Struct):
    pass


class SourceModel(Struct):
    pass


class ParametersModel(Struct):
    pass


class TaskModel():
    def __int__(self, sinks: [SinkModel], sources: [SourceModel], parameters: ParametersModel):
        self._sinks = sinks
        self._sources = sources
        self._parameters = parameters

    @property
    def sinks(self) -> [SinkModel]:
        return self._sinks

    @property
    def sources(self) -> [SourceModel]:
        return self._sources

    @property
    def parameters(self)-> ParametersModel:
        return self._parameters


class PipelineModel:
