# Copyright (c) Alibaba, Inc. and its affiliates.
import gi
from .base_pipeline import BasePipeline
from .gstreamer_pipeline import GStreamerPipeline
from ..model.struct import Struct

gi.require_version("Gst", "1.0")
gi.require_version("GstApp", "1.0")
gi.require_version("GstVideo", "1.0")
from gi.repository import Gst, GLib, GObject, GstApp, GstVideo


class DelegateGStreamerPipeline(BasePipeline):

    def __init__(self, delegate: GStreamerPipeline) -> None:
        super().__init__()
        self._delegate = delegate
        self._delegate.set_pipeline_configure(self.configure_pipeline)

    @property
    def delegate(self):
        """delegated pipeline object"""
        return self._delegate

    def startup(self) -> None:
        self._delegate.startup()

    def shutdown(self, timeout: int =1, eos: bool = False) -> None:
        self._delegate.shutdown()

    @property
    def is_active(self) -> bool:
        return self._delegate.is_active

    @property
    def is_done(self) -> bool:
        return self._delegate.is_done

    @property
    def log(self):
        return self.delegate.log

    def configure_pipeline(self, gst_pipeline: Gst.Pipeline):
        pass
