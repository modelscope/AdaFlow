from typing import Dict

from ..pipeline_composer import PipelineComposer
from ..model.task import Task
from abc import ABCMeta, abstractmethod
from ..model.pipeline import Pipeline


class BasePipeline(metaclass=ABCMeta):
    """
    Backend specific pipeline handler
    """
    def __init__(self, pipeline: Pipeline) -> None:
        super().__init__()
        self._pipeline = pipeline

    @property
    def pipeline(self):
        return self._pipeline

    @abstractmethod
    def startup(self, task: Dict[str, any]):
        pass

    @abstractmethod
    def stop(self):
        pass

    @abstractmethod
    def shutdown(self):
        pass

    @property
    def is_active(self) -> bool:
        pass

    @property
    def is_done(self) -> bool:
        pass

    @abstractmethod
    def push(self, src_name: str, data_packet):
        """
        push data to named appsrc
        :param src_name:
        :param data_packet
        :return:
        """
        pass

    @abstractmethod
    def pop(self, sink_name: str):
        """
        add callback for appsink data
        :param sink_name:
        :return:
        """
        pass

