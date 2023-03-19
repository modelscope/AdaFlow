from typing import Dict

from ..pipeline_composer import PipelineComposer

from abc import ABCMeta, abstractmethod
from ..model.struct import Struct


class BasePipeline(metaclass=ABCMeta):
    """
    Backend specific pipeline handler
    """
    def __init__(self) -> None:
        super().__init__()

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


