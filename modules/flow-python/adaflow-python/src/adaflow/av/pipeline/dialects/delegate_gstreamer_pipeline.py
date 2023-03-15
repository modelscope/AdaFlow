import gi
from ..model.pipeline import Pipeline
from ..model.task import Task
from .base_pipeline import BasePipeline
from .gstreamer_pipeline import GStreamerPipeline

gi.require_version("Gst", "1.0")
gi.require_version("GstApp", "1.0")
gi.require_version("GstVideo", "1.0")
from gi.repository import Gst, GLib, GObject, GstApp, GstVideo


class DelegateGStreamerPipeline(BasePipeline):

    def __init__(self, delegate: GStreamerPipeline) -> None:
        super().__init__()
        self._delegate = delegate
        self._delegate.set_pipeline_configurer(self.configure_pipeline)

    @property
    def delegate(self):
        return self._delegate

    @property
    def pipeline(self) -> Pipeline:
        return self._delegate.pipeline

    @property
    def task(self) -> Task:
        return self._delegate.task

    def startup(self) -> None:
        self._delegate.startup()

    def shutdown(self) -> None:
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
