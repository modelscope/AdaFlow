# Copyright (c) Alibaba, Inc. and its affiliates.
import queue

from .delegate_gstreamer_pipeline import DelegateGStreamerPipeline
from .gstreamer_pipeline import GStreamerPipeline, GStreamerPipelineBuilder
from adaflow.av.data.av_data_packet import AVDataPacket

import typing as typ
import gi
gi.require_version("Gst", "1.0")
gi.require_version("GstApp", "1.0")
gi.require_version("GstVideo", "1.0")
from gi.repository import Gst, GLib, GObject, GstApp, GstVideo  # noqa:F401,F402


class ReadableGStreamerPipeline(DelegateGStreamerPipeline):
    def __init__(self, delegate: GStreamerPipeline, max_buffers_size: int = 100) -> None:
        super().__init__(delegate)
        self._sink = None  # GstApp.AppSink
        self._counter = 0  # counts number of received buffers
        self._queue = queue.Queue(maxsize=max_buffers_size)

    @property
    def total_buffers_count(self) -> int:
        """Total read buffers count """
        return self._counter

    def configure_pipeline(self, gst_pipeline: Gst.Pipeline):
        appsinks = self.delegate.get_elements_by_class(GstApp.AppSink)
        self._sink = appsinks[0] if len(appsinks) == 1 else None
        if not self._sink:
            raise AttributeError("%s not found", GstApp.AppSink)

        # Listen to 'new-sample' event
        # https://lazka.github.io/pgi-docs/GstApp-1.0/classes/AppSink.html#GstApp.AppSink.signals.new_sample
        if self._sink:
            self._sink.connect("new-sample", self._on_buffer, None)

    def _on_buffer(self, sink: GstApp.AppSink, data: typ.Any) -> Gst.FlowReturn:
        """Callback on 'new-sample' signal"""
        # Emit 'pull-sample' signal
        # https://lazka.github.io/pgi-docs/GstApp-1.0/classes/AppSink.html#GstApp.AppSink.signals.pull_sample

        sample = sink.emit("pull-sample")
        if isinstance(sample, Gst.Sample):
            data_packet = self._extract_buffer(sample)
            if data_packet:
                self._queue.put(data_packet)
                self._counter += 1

            return Gst.FlowReturn.OK

        self.log.error(
            "Error : Not expected buffer type: %s != %s. %s",
            type(sample),
            Gst.Sample,
            self,
        )
        return Gst.FlowReturn.ERROR

    def _extract_buffer(self, sample: Gst.Sample) -> [AVDataPacket, None]:
        buffer = sample.get_buffer()
        caps = sample.get_caps()

        cnt = buffer.n_memory()
        if cnt <= 0:
            self.log.warning("%s No data in Gst.Buffer", self)
            return None

        memory = buffer.get_memory(0)
        if not memory:
            self.log.warning("%s No Gst.Memory in Gst.Buffer", self)
            return None

        return AVDataPacket(buffer, caps)

    def _clean_queue(self, q: queue.Queue):
        while not q.empty():
            try:
                q.get_nowait()
            except queue.Empty:
                break

    @property
    def queue_size(self) -> int:
        """Returns queue size of GstBuffer"""
        return self._queue.qsize()

    def shutdown(self, timeout: int = 1, eos: bool = False):
        super().shutdown()
        self._clean_queue(self._queue)

    def pop(self, timeout: float = 0.1) -> AVDataPacket:
        """ Pops AVDataPacket """
        if not self._sink:
            raise RuntimeError("Sink is not initialized for pipeline %s" % self)

        buffer = None
        while (self.is_active or not self._queue.empty()) and not buffer:
            try:
                buffer = self._queue.get(timeout=timeout)
            except queue.Empty:
                pass

        return buffer


class ReadableGStreamerPipelineBuilder(GStreamerPipelineBuilder):

    def __init__(self) -> None:
        super().__init__()
        self._max_buffers_size = 100

    def max_buffers_size(self, size: int = 100):
        self._max_buffers_size = size
        return self

    def build(self) -> ReadableGStreamerPipeline:
        return ReadableGStreamerPipeline(GStreamerPipeline(self._pipeline, self._task), self._max_buffers_size)


