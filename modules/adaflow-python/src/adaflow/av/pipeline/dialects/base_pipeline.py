# Copyright (c) Alibaba, Inc. and its affiliates.
from typing import Dict

from ..pipeline_composer import PipelineComposer

from abc import ABCMeta, abstractmethod
from ..model.struct import Struct


class BasePipeline(metaclass=ABCMeta):
    """Backend specific pipeline handler
    """

    def __init__(self) -> None:
        super().__init__()

    def __enter__(self):
        self.startup()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.shutdown()
        return False

    @abstractmethod
    def startup(self) -> None:
        """Abstract method to start this pipeline. Subclass should have implmentation to initialize and start the pipeline.

        Returns:
            nothing
        """
        pass

    @abstractmethod
    def shutdown(self, timeout: int =1, eos: bool = False) -> None:
        """ Abstract method to terminate this pipeline. Subclass should have implmentation to cleanup internal states
        and shutdown.

        Args:
            timeout: interval for forcibly sthudown timeout
            eos: flag to indcate if a End-of-Stream event should be fired

        Returns:
            nothing
        """
        pass

    @property
    @abstractmethod
    def is_active(self) -> bool:
        """a boolean flag to indicate if this pipeline is actively running
        """
        pass

    @property
    @abstractmethod
    def is_done(self) -> bool:
        """a boolean flag to indicate if this pipeline is completed. Subclass should have implmentation to garantee
        that this function always return `True` if the pipeline is either successfully finished or failing with
        exception. As a significant state indicator, this property is often used in a while loop to optimisitically
        wait for completion of a pipeline. e.g:

        Examples:
            import time

            p = <Pipeline instance>
            while not p.is_done:
                time.sleep(1)

        """
        pass


