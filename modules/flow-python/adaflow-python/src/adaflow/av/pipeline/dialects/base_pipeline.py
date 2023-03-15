from typing import Dict

from ..pipeline_composer import PipelineComposer
from ..model.task import Task
from abc import ABCMeta, abstractmethod
from ..model.pipeline import Pipeline


class BasePipeline(metaclass=ABCMeta):
    """
    Backend specific pipeline handler
    """
    def __init__(self) -> None:
        super().__init__()

    @property
    @abstractmethod
    def pipeline(self) -> Pipeline:
        pass

    @property
    @abstractmethod
    def task(self) -> Task:
        pass

    @abstractmethod
    def startup(self):
        pass

    @abstractmethod
    def shutdown(self):
        pass

    @property
    @abstractmethod
    def is_active(self) -> bool:
        pass

    @property
    @abstractmethod
    def is_done(self) -> bool:
        pass


