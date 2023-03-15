from adaflow.av.pipeline.dialects.gstreamer_pipeline import GStreamerPipeline
from delegate_gstreamer_pipeline import DelegateGStreamerPipeline
import gi
import typing as typ
from readable_gstreamer_pipeline import ReadableGStreamerPipeline
from writable_gstreamer_pipeline import WritableGstreamerPipeline
from gst_tools import VideoType, gst_video_format_plugin, to_gst_buffer
from fractions import Fraction
import numpy as np

gi.require_version("Gst", "1.0")
gi.require_version("GstApp", "1.0")
gi.require_version("GstVideo", "1.0")
from gi.repository import Gst, GLib, GObject, GstApp, GstVideo


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

