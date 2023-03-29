# Copyright (c) Alibaba, Inc. and its affiliates.
from adaflow.av.pipeline.dialects.gstreamer_pipeline import GStreamerPipeline, GStreamerPipelineBuilder
from .delegate_gstreamer_pipeline import DelegateGStreamerPipeline

import typing as typ
from .readable_gstreamer_pipeline import ReadableGStreamerPipeline
from .writable_gstreamer_pipeline import WritableGstreamerPipeline
from .gst_tools import VideoType, gst_video_format_plugin, to_gst_buffer
from fractions import Fraction
import numpy as np

import gi
gi.require_version("Gst", "1.0")
gi.require_version("GstApp", "1.0")
gi.require_version("GstVideo", "1.0")
from gi.repository import Gst, GLib, GObject, GstApp, GstVideo # noqa:F401,F402


class DuplexGstreamerPipeline(DelegateGStreamerPipeline):
    def __init__(self,
                 delegate: GStreamerPipeline,
                 width: int,
                 height: int,
                 fps: typ.Union[Fraction, int] = Fraction("30/1"),
                 video_type: VideoType = VideoType.VIDEO_RAW,
                 video_frmt: GstVideo.VideoFormat = GstVideo.VideoFormat.RGB,
                 max_buffers_size: int = 100,
                 ) -> None:
        super().__init__(delegate)
        self._readable = ReadableGStreamerPipeline(delegate, max_buffers_size)
        self._writable = WritableGstreamerPipeline(delegate, width, height, fps, video_type, video_frmt)
        delegate.set_pipeline_configure(self.configure_pipeline)

    def push(self,
             buffer: typ.Union[Gst.Buffer, np.ndarray],
             *,
             pts: typ.Optional[int] = None,
             dts: typ.Optional[int] = None,
             offset: typ.Optional[int] = None
             ):
        self._writable.push(buffer, pts=pts, dts=dts, offset=offset)

    def pop(self, timeout: float = 0.1):
        return self._readable.pop(timeout=timeout)

    def end(self):
        self._writable.end()

    def configure_pipeline(self, gst_pipeline: Gst.Pipeline):
        self._readable.configure_pipeline(gst_pipeline)
        self._writable.configure_pipeline(gst_pipeline)


class DuplexGstreamerPipelineBuilder(GStreamerPipelineBuilder):

    def __init__(self) -> None:
        super().__init__()
        self._max_buffers_size = 100
        self._width = None
        self._height = None
        self._fps = Fraction("30/1")
        self._video_type = VideoType.VIDEO_RAW
        self._video_frmt = GstVideo.VideoFormat.RGB

    def caps_filter(self,
                    width: int,
                    height: int,
                    fps: typ.Union[Fraction, int] = Fraction("30/1"),
                    video_type: VideoType = VideoType.VIDEO_RAW,
                    video_frmt: GstVideo.VideoFormat = GstVideo.VideoFormat.RGB):
        self._width = width
        self._height = height
        self._fps = fps
        self._video_type = video_type
        self._video_frmt = video_frmt
        return self

    def max_buffers_size(self, size: int = 100):
        self._max_buffers_size = size
        return self

    def build(self) -> DuplexGstreamerPipeline:
        assert self._width
        assert self._height
        return DuplexGstreamerPipeline(
            GStreamerPipeline(self._pipeline, self._task),
            self._width,
            self._height,
            self._fps,
            self._video_type,
            self._video_frmt,
            self._max_buffers_size
        )
