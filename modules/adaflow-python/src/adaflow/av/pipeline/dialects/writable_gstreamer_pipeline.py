# Copyright (c) Alibaba, Inc. and its affiliates.
from .delegate_gstreamer_pipeline import DelegateGStreamerPipeline
from .gstreamer_pipeline import GStreamerPipeline, GStreamerPipelineBuilder
from adaflow.av.data.av_data_packet import AVDataPacket
import typing as typ
from fractions import Fraction

import numpy as np
from .gst_tools import VideoType, gst_video_format_plugin, to_gst_buffer

import gi
gi.require_version("Gst", "1.0")
gi.require_version("GstApp", "1.0")
gi.require_version("GstVideo", "1.0")
from gi.repository import Gst, GLib, GObject, GstApp, GstVideo  # noqa:F401,F402

class WritableGstreamerPipeline(DelegateGStreamerPipeline):

    def __init__(self,
                 delegate: GStreamerPipeline,
                 width: int,
                 height: int,
                 fps: typ.Union[Fraction, int] = Fraction("30/1"),
                 video_type: VideoType = VideoType.VIDEO_RAW,
                 video_format: GstVideo.VideoFormat = GstVideo.VideoFormat.RGB
                 ) -> None:
        super().__init__(delegate)
        self._fps = Fraction(fps)
        self._width = width
        self._height = height
        self._video_type = video_type  # VideoType
        self._video_format = video_format  # GstVideo.VideoFormat

        self._pts = 0
        self._dts = GLib.MAXUINT64
        self._duration = 10 ** 9 / (fps.numerator / fps.denominator)

        self._src = None  # GstApp.AppSrc

    @property
    def video_format(self):
        return self.video_format

    def push(
            self,
            buffer: typ.Union[Gst.Buffer, np.ndarray, AVDataPacket],
            *,
            pts: typ.Optional[int] = None,
            dts: typ.Optional[int] = None,
            offset: typ.Optional[int] = None
    ) -> None:
        if not self.is_active:
            self.log.warning("Warning %s: Can't push buffer. Pipeline not active")
            return

        if not self._src:
            raise RuntimeError("Src {} is not initialized".format(Gst.AppSrc))

        self._pts += self._duration
        offset_ = int(self._pts / self._duration)

        gst_buffer = to_gst_buffer(
            buffer,
            pts=pts or self._pts,
            dts=dts or self._dts,
            offset=offset or offset_,
            duration=self._duration,
        )

        # Emit 'push-buffer' signal
        # https://lazka.github.io/pgi-docs/GstApp-1.0/classes/AppSrc.html#GstApp.AppSrc.signals.push_buffer
        self._src.emit("push-buffer", gst_buffer)

    def end(self):
        """
        send EOS to appsrc element, which popagates to downstreams
        :return:
        """
        self.log.info("send EOS")
        self._src.emit("end-of-stream")

    def configure_pipeline(self, gst_pipeline: Gst.Pipeline):
        # find src element
        appsrcs = self.delegate.get_elements_by_class(GstApp.AppSrc)
        self._src = appsrcs[0] if len(appsrcs) == 1 else None
        if not self._src:
            raise ValueError("%s not found", GstApp.AppSrc)

        if self._src:
            # this instructs appsrc that we will be dealing with timed buffer
            self._src.set_property("format", Gst.Format.TIME)

            # instructs appsrc to block pushing buffers until ones in queue are preprocessed
            # allows to avoid huge queue internal queue size in appsrc
            self._src.set_property("block", True)

            # set src caps
            caps = gst_video_format_plugin(
                width=self._width,
                height=self._height,
                fps=self._fps,
                video_type=self._video_type,
                video_frmt=self._video_format,
            )

            self.log.debug("%s Caps: %s", self, caps)
            if caps is not None:
                self._src.set_property("caps", Gst.Caps.from_string(caps))


class WritableGstreamerPipelineBuilder(GStreamerPipelineBuilder):
    def __init__(self) -> None:
        super().__init__()
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

    def build(self) -> WritableGstreamerPipeline:
        assert self._width
        assert self._height
        return WritableGstreamerPipeline(
            GStreamerPipeline(self._pipeline, self._task),
            self._width,
            self._height,
            self._fps,
            self._video_type,
            self._video_frmt
        )
