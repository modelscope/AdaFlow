import threading
from typing import Dict, Callable, List


from .base_pipeline import BasePipeline
from ..model.struct import Struct
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
from gi.repository import Gst, GLib, GObject, GstApp, GstVideo


class GStreamerPipeline(BasePipeline):
    def __init__(self, pipeline_model: Dict[str, any], task_model: Dict[str, any], pipeline_configure: Callable[[BasePipeline], None] = None) -> None:
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

    @property
    def log(self) -> logging.Logger:
        return self._log

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        return False

    def set_pipeline_configure(self, pipeline_configure: Callable[[BasePipeline], None]):
        self._pipeline_configure = pipeline_configure

    def startup(self):
        # validate parameters before startup
        validate(
            instance=self.parameters,
            schema=self._pipeline_model["parameters_schema"],
            # https://python-jsonschema.readthedocs.io/en/latest/validate/#validating-formats
            format_checker=Draft202012Validator.FORMAT_CHECKER
        )
        # v = Draft202012Validator(self._pipeline_model["parameters"])
        # for error in sorted(v.iter_errors(self.parameters), key=str):
        #     self._log.error("validation error %s" % error.message)

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
        return "GStreamerPipeline[name=%s]" % self._pipeline_model["name"]

    @property
    def command(self):
        return self._template.render({
            "parameters": self.parameters
        })

    @property
    def parameters(self) -> Dict[str, any]:
        return self._parameters

    @property
    def is_active(self) -> bool:
        return self._gst_pipeline is not None

    @property
    def is_done(self) -> bool:
        return self._terminal_event.is_set()

    def _evaluate_parameters(self):
        parameters = dict(self._task_model.get("parameters", {}))
        if self._pipeline_model["parameters_schema"] is None:
            self._log.debug("skip json validation because no schema is given in pipeline DSL")
            return parameters
        parameter_schema = self._pipeline_model["parameters_schema"]
        Draft202012Validator.check_schema(parameter_schema)
        assert parameter_schema["type"] == "object"
        for k, v_schema in parameter_schema["properties"].items():
            if "default" in v_schema:
                if k not in parameters or parameters[k] is None or parameters[k] == '':
                    parameters[k] = self._template_env.from_string(
                        str(v_schema["default"]),
                        {"envs": os.environ}).render()
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

    def __init__(self) -> None:
        super().__init__()
        self._pipeline = None
        self._task = None

    def pipeline(self, pipeline_model: Struct):
        self._pipeline = pipeline_model
        return self

    def task(self, task_model: Struct):
        self._task = task_model
        return self

    def build(self) -> GStreamerPipeline:
        return GStreamerPipeline(self._pipeline, self._task)

