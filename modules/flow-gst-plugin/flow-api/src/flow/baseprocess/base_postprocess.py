import abc
from typing import Any, Dict


class PostprocessABC(metaclass=abc.ABCMeta):
    ''' post process abstract class'''

    def __init__(self):
        pass

    @abc.abstractmethod
    def parse_input(self, input: str):
        raise NotImplementedError()

    @abc.abstractmethod
    def videoinfo(self, **kwargs):
        raise NotImplementedError()

    @abc.abstractmethod
    def process(self, metadata, image):
        raise NotImplementedError()


class BasePostprocess(PostprocessABC):
    ''' base class of post process class'''

    def __init__(self):
        super().__init__()

    def videoinfo(self, **kwargs):
        pass

    def parse_input(self, input: str):
        pass

    def process(self, metadata, image):
        pass
