# Copyright (c) Alibaba, Inc. and its affiliates.
import typing as typ
from fractions import Fraction
from enum import Enum
from adaflow.av.data.av_data_packet import AVDataPacket
import numpy as np
import gi
gi.require_version("Gst", "1.0")
gi.require_version("GstApp", "1.0")
gi.require_version("GstVideo", "1.0")
from gi.repository import Gst, GLib, GObject, GstApp, GstVideo  # noqa:F401,F402



def fraction_to_str(fraction: Fraction) -> str:
    """Converts fraction to str"""
    return '{}/{}'.format(fraction.numerator, fraction.denominator)


class NamedEnum(Enum):
    def __repr__(self):
        return str(self)

    @classmethod
    def names(cls) -> typ.List[str]:
        return list(cls.__members__.keys())


class VideoType(NamedEnum):
    """
    https://gstreamer.freedesktop.org/documentation/plugin-development/advanced/media-types.html?gi-language=c
    """

    VIDEO_RAW = "video/x-raw"
    VIDEO_GL_RAW = "video/x-raw(memory:GLMemory)"
    VIDEO_NVVM_RAW = "video/x-raw(memory:NVMM)"


def gst_video_format_plugin(
        *,
        width: int = None,
        height: int = None,
        fps: Fraction = None,
        video_type: VideoType = VideoType.VIDEO_RAW,
        video_frmt: GstVideo.VideoFormat = GstVideo.VideoFormat.RGB
) -> typ.Optional[str]:
    """
        https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer-plugins/html/gstreamer-plugins-capsfilter.html
        Returns capsfilter
            video/x-raw,width=widht,height=height
            video/x-raw,framerate=fps/1
            video/x-raw,format=RGB
            video/x-raw,format=RGB,width=widht,height=height,framerate=1/fps
        :param width: image width
        :param height: image height
        :param fps: video fps
        :param video_type: gst specific (raw, h264, ..)
            https://gstreamer.freedesktop.org/documentation/design/mediatype-video-raw.html
        :param video_frmt: gst specific (RGB, BGR, RGBA)
            https://gstreamer.freedesktop.org/documentation/design/mediatype-video-raw.html
            https://lazka.github.io/pgi-docs/index.html#GstVideo-1.0/enums.html#GstVideo.VideoFormat
    """

    plugin = str(video_type.value)
    n = len(plugin)
    if video_frmt:
        plugin += ",format={}".format(GstVideo.VideoFormat.to_string(video_frmt))
    if width and width > 0:
        plugin += ",width={}".format(width)
    if height and height > 0:
        plugin += ",height={}".format(height)
    if fps and fps > 0:
        plugin += ",framerate={}".format(fraction_to_str(fps))

    if n == len(plugin):
        return None

    return plugin


def to_gst_buffer(
        buffer: typ.Union[Gst.Buffer, np.ndarray, AVDataPacket],
        *,
        pts: typ.Optional[int] = None,
        dts: typ.Optional[int] = None,
        offset: typ.Optional[int] = None,
        duration: typ.Optional[int] = None
) -> Gst.Buffer:
    """Convert buffer to Gst.Buffer. Updates required fields
    Parameters explained:
        https://lazka.github.io/pgi-docs/Gst-1.0/classes/Buffer.html#gst-buffer
    """
    gst_buffer = buffer
    if isinstance(gst_buffer, np.ndarray):
        gst_buffer = Gst.Buffer.new_wrapped(bytes(buffer))

    if isinstance(buffer, AVDataPacket):
        gst_buffer = buffer[0].buffer

    if not isinstance(gst_buffer, Gst.Buffer):
        raise ValueError(
            "Invalid buffer format {} != {}".format(type(gst_buffer), Gst.Buffer)
        )

    gst_buffer.pts = pts or GLib.MAXUINT64
    gst_buffer.dts = dts or GLib.MAXUINT64
    gst_buffer.offset = offset or GLib.MAXUINT64
    gst_buffer.duration = duration or GLib.MAXUINT64
    return gst_buffer
