# Copyright (c) Alibaba, Inc. and its affiliates.
import sys
import threading
from typing import Dict, Callable, List
from .base_pipeline import BasePipeline
import os
from jinja2 import Environment
from .dialect_template_helper import GStreamerTemplateHelper
import logging
from jsonschema import validate
from jsonschema.validators import Draft202012Validator

import gi
gi.require_version("Gst", "1.0")
gi.require_version("GstApp", "1.0")
gi.require_version("GstVideo", "1.0")
from gi.repository import Gst, GLib, GObject, GstApp, GstVideo  # noqa:F401,F402


Gst.init(sys.argv if hasattr(sys, "argv") else None)


class GStreamerPipeline(BasePipeline):
    def __init__(self, pipeline_model: Dict[str, any], task_model: Dict[str, any], pipeline_configure: Callable[[Gst.Pipeline], None] = None) -> None:
        """
        Default constructor for a plain GStreamerPipeline
        Args:
            pipeline_model: Pipeline definition dict object
            task_model: Task definition dict object
            pipeline_configure: Pipeline configuration function
        """
        # TODO validate pipeline_model and task_model
        super().__init__()
        self._pipeline_model = pipeline_model
        self._task_model = task_model
        self._bus = None
        self._gst_pipeline = None
        self._log = logging.getLogger("GStreamerPipeline")
        self._terminal_event = threading.Event()
        self._template_env = Environment()

        # convert array to string
        template_string = pipeline_model["dialect"]
        if isinstance(template_string, list):
            template_string = " ! ".join(template_string)

        self._template = self._template_env.from_string(template_string, {"F": GStreamerTemplateHelper(task_model)})
        self._pipeline_configure = pipeline_configure

        # render parameters
        self._parameters = self._evaluate_parameters()

        self.log.debug("init pipeline with command %s" % self.command)

    @property
    def log(self) -> logging.Logger:
        return self._log

    def set_pipeline_configure(self, pipeline_configure: Callable[[Gst.Pipeline], None]):
        """
        setter for pipeline configuration function.

        Args:
            pipeline_configure:

        Returns:

        """
        self._pipeline_configure = pipeline_configure

    def startup(self):
        self.log.debug("starting pipeline %s", self)
        self._gst_pipeline = Gst.parse_launch(self.command)

        self.log.debug("set pipeline %s to READY", self)

        # Initialize
        self._bus = self._gst_pipeline.get_bus()
        self._bus.add_signal_watch()
        self._bus.connect("message::error", self.on_error)
        self._bus.connect("message::eos", self.on_eos)
        self._bus.connect("message::warning", self.on_warning)
        if self._pipeline_configure is not None:
            self._pipeline_configure(self._gst_pipeline)
        self._gst_pipeline.set_state(Gst.State.READY)
        self._terminal_event.clear()

        # Start
        self.log.debug("set pipeline %s to PLAYING", self)
        self._gst_pipeline.set_state(Gst.State.PLAYING)

    def __str__(self):
        return "GStreamerPipeline[name=%s]" % self._pipeline_model["name"]

    @property
    def command(self):
        """final command after parameter substitutions"""
        return self._template.render({
            "parameters": self.parameters
        })

    @property
    def parameters(self) -> Dict[str, any]:
        """resolved parameter dict"""
        return self._parameters

    @property
    def is_active(self) -> bool:
        return self._gst_pipeline is not None

    @property
    def is_done(self) -> bool:
        return self._terminal_event.is_set()

    def _evaluate_parameters(self):
        parameters = dict(self._task_model.get("parameters", {}))
        if "parameters_schema" not in self._pipeline_model or self._pipeline_model["parameters_schema"] is None:
            self._log.debug("skip json validation because no schema is given in pipeline DSL")
            return parameters
        parameter_schema = self._pipeline_model["parameters_schema"]
        Draft202012Validator.check_schema(parameter_schema)
        assert parameter_schema["type"] == "object"
        for k, v_schema in parameter_schema["properties"].items():
            if "default" in v_schema:
                if k not in parameters or parameters[k] is None or parameters[k] == '':
                    if isinstance(v_schema["default"], str):
                        parameters[k] = self._template_env.from_string(v_schema["default"],
                        {"envs": os.environ}).render()
                    else:
                        parameters[k] = v_schema["default"]
        # validate parameters before startup
        validate(
            instance=parameters,
            schema=self._pipeline_model["parameters_schema"],
            # https://python-jsonschema.readthedocs.io/en/latest/validate/#validating-formats
            format_checker=Draft202012Validator.FORMAT_CHECKER
        )
        return parameters

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

        if not self._pipeline_model:
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
    """Builder for GStreamerPipeline"""

    def __init__(self) -> None:
        super().__init__()
        self._pipeline = {}
        self._task = {}

    def pipeline(self, pipeline_model: Dict[str, any]):
        """set pipeline model dict"""
        self._pipeline = pipeline_model
        return self

    def task(self, task_model: Dict[str, any]):
        """set task model dict"""
        self._task = task_model
        return self

    def sink(self, sink_model: Dict[str, any]):
        """append a `sink model` dict to task definition model. `task model` data will be initialized as `list` if not already set"""
        if "sinks" not in self._task or self._task["sinks"] is None:
            self._task["sinks"] = []
        self._task["sinks"].append(sink_model)
        return self

    def source(self, source_model: Dict[str, any]):
        """append a `source model` dict to task definition model. `task model` data will be initialized as `list` if not already set"""
        if "sources" not in self._task or self._task["sources"] is None:
            self._task["sources"] = []
        self._task["sources"].append(source_model)
        return self

    def parameter(self, parameters: Dict[str, any]):
        """
        set `parameter` data of task definition

        Args:
            parameters: dict for parameter values

        Returns:
            self reference

        """
        if "parameters" not in self._task or parameters is not None:
            self._task["parameters"] = parameters
        return self

    def build(self) -> GStreamerPipeline:
        """Return the pipeline object"""
        return GStreamerPipeline(self._pipeline, self._task)

