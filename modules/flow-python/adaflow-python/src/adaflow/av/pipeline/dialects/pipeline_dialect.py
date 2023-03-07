from typing import Dict

from ..pipeline_composer import PipelineComposer
from ..model.task_definition import TaskDefinition
from abc import ABCMeta, abstractmethod


class PipelineDialect(metaclass=ABCMeta):
    """
    Backend specific pipeline handler
    """
    def __init__(self, pipeline_composer: PipelineComposer) -> None:
        super().__init__()
        self._pipeline = pipeline_composer

    @property
    def pipeline(self):
        return self._pipeline

    @abstractmethod
    def execute(self, task: TaskDefinition) -> any:
        """
        translate JSON DSL to dialect DSL
        :return:
        """
        pass

