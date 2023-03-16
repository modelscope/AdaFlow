import threading
from typing import Dict, Callable, List

import networkx as nx
from adaflow.av.pipeline import PipelineComposer
from .base_pipeline import BasePipeline
from ..model.pipeline import Pipeline
from ..model.task import Task
from abc import ABCMeta, abstractmethod
import os
from jinja2 import Template, Environment

import gi
gi.require_version("Gst", "1.0")
gi.require_version("GstApp", "1.0")
gi.require_version("GstVideo", "1.0")
from gi.repository import Gst, GLib, GObject, GstApp, GstVideo
import logging
from dialect_template_helper import GStreamerTemplateHelper
import gst_tools


class GStreamerPipeline(BasePipeline):
    def __init__(self, pipeline: Pipeline, task: Task, pipeline_configure: Callable[[BasePipeline], None]=None) -> None:
        super().__init__()
        self._pipeline = pipeline
        self._task = task
        self._bus = None
        self._gst_pipeline = None
        self._log = logging.getLogger("GStreamerPipeline")
        self._terminal_event = threading.Event()
        self._template_env = Environment()
        self._template = self._template_env.from_string(pipeline.dialect, {"F": GStreamerTemplateHelper(task)})
        self._pipeline_configure = pipeline_configure

    @property
    def log(self) -> logging.Logger:
        return self._log

    def set_pipeline_configure(self, pipeline_configure: Callable[[BasePipeline], None]):
        self._pipeline_configure = pipeline_configure

    def startup(self):
        self.log.debug("starting pipeline %s", self)
        self._gst_pipeline = Gst.parse_launch(self.command)


        self.log.debug("set pipeline %s to READY", self)
        self._gst_pipeline.set_state(Gst.State.READY)
        self._terminal_event.clear()

        # Initialize
        self._bus = self._gst_pipeline.get_bus()
        self._bus.add_signal_watch()
        self._bus.connect("message::error", self.on_error)
        self._bus.connect("message::eos", self.on_eos)
        self._bus.connect("message::warning", self.on_warning)

        if self._pipeline_configure is not None:
            self._pipeline_configure(self._gst_pipeline)

        self.log.debug("set pipeline %s to PLAYING", self)
        self._gst_pipeline.set_state(Gst.State.PLAYING)

    def __str__(self):
        return "GStreamerPipeline[name=%s]" % self._pipeline.name

    @property
    def pipeline(self) -> Pipeline:
        return self._pipeline

    @property
    def task(self) -> Task:
        return self._task

    @property
    def command(self):
        return self._template.render({
            "parameters": self.parameters
        })

    @property
    def parameters(self) -> Dict[str, any]:
        assert self._pipeline.parameters.type == "object"
        parameters = self._task.parameters.copy()
        for k, v_schema in self.pipeline.parameters.properties:
            if k not in parameters and v_schema.default:
                parameters[k] = self._template_env.from_string(
                    v_schema.default,
                    {
                        "envs": os.environ
                    }).render()
        return parameters

    @property
    def is_active(self) -> bool:
        return self._gst_pipeline is not None

    @property
    def is_done(self) -> bool:
        return self._terminal_event.is_set()

    def shutdown(self, timeout: int =1, eos: bool = False) -> None:
        self.log.info("about to shutdown %s" % self)
        self._shutdown_pipeline(timeout=timeout, eos=eos)
        self.log.info("successfully shutdown %s" % self)

    def on_error(self, bus: Gst.Bus, message: Gst.Message):
        err, debug = message.parse_error()
        self.log.error("Gstreamer.%s: Error %s: %s. ", self, err, debug)
        self._shutdown_pipeline()

    def on_eos(self, bus: Gst.Bus, message: Gst.Message):
        self.log.debug("Gstreamer.%s: Received stream EOS event", self)
        self._shutdown_pipeline()

    def on_warning(self, bus: Gst.Bus, message: Gst.Message):
        warn, debug = message.parse_warning()
        self.log.warning("Gstreamer.%s: %s. %s", self, warn, debug)

    def get_elements_by_class(self, cls: GObject.GType) -> List[Gst.Element]:
        """ Get Gst.Element[] from pipeline by GType """
        elements = self._gst_pipeline.iterate_elements()
        if isinstance(elements, Gst.Iterator):
            # Patch "TypeError: ‘Iterator’ object is not iterable."
            # For versions, we have to get a python iterable object from Gst iterator
            _elements = []
            while True:
                ret, el = elements.next()
                if ret == Gst.IteratorResult(1):  # GST_ITERATOR_OK
                    _elements.append(el)
                else:
                    break
            elements = _elements

        return [e for e in elements if isinstance(e, cls)]

    def get_element_by_name(self, name: str) -> Gst.Element:
        """Get Gst.Element from pipeline by name
        :param name: plugins name (name={} in gst-launch string)
        """
        return self._gst_pipeline.get_by_name(name)


    def _shutdown_pipeline(self, timeout: int = 1, eos: bool = False) -> None:
        """ Stops pipeline
        :param eos: if True -> send EOS event
            - EOS event necessary for FILESINK finishes properly
            - Use when pipeline crushes
        """
        if self._terminal_event.is_set():
            return

        self._terminal_event.set()

        if not self.pipeline:
            return

        self.log.debug("%s Stopping pipeline ...", self)
        if self._gst_pipeline.get_state(timeout=1)[1] == Gst.State.PLAYING:
            self.log.debug("%s Sending EOS event ...", self)
            try:
                thread = threading.Thread(
                    target=self._gst_pipeline.send_event, args=(Gst.Event.new_eos(),)
                )
                thread.start()
                thread.join(timeout=timeout)
            except Exception:
                pass

        self.log.debug("%s Reseting pipeline state ....", self)
        try:
            self._gst_pipeline.set_state(Gst.State.NULL)
            self._gst_pipeline = None
        except Exception:
            pass

        self.log.debug("%s Gst.Pipeline successfully destroyed", self)


class GStreamerPipelineBuilder:

    def __init__(self) -> None:
        super().__init__()
        self._pipeline = None
        self._task = None

    def pipeline(self, pipeline_model: Pipeline):
        self._pipeline = pipeline_model
        return self

    def task(self, task_model: Task):
        self._task = task_model
        return self

    def build(self) -> GStreamerPipeline:
        return GStreamerPipeline(self._pipeline, self._task)

