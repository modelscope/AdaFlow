# Copyright (c) Alibaba, Inc. and its affiliates.
import math
import json
import numpy as np
import gi
gi.require_version('GstVideo', '1.0')
from gi.repository import GstVideo

BITS_PER_BYTE = 8

_ALL_VIDEO_FORMATS = [GstVideo.VideoFormat.from_string(
    f.strip()) for f in GstVideo.VIDEO_FORMATS_ALL.strip('{ }').split(',')]


def has_flag(value: GstVideo.VideoFormatFlags,
             flag: GstVideo.VideoFormatFlags) -> bool:
    # in VideoFormatFlags each new value is 1 << 2**{0...8}
    return bool(value & (1 << max(1, math.ceil(math.log2(int(flag))))))


def _get_num_channels(fmt: GstVideo.VideoFormat) -> int:
    """
        -1: means complex format (YUV, ...)
    """

    frmt_info = GstVideo.VideoFormat.get_info(fmt)
    # temporal fix
    if fmt == GstVideo.VideoFormat.BGRX:
        return 4

    if has_flag(frmt_info.flags, GstVideo.VideoFormatFlags.ALPHA):
        return 4

    if has_flag(frmt_info.flags, GstVideo.VideoFormatFlags.RGB):
        return 3

    if has_flag(frmt_info.flags, GstVideo.VideoFormatFlags.GRAY):
        return 1

    return -1


_ALL_VIDEO_FORMAT_CHANNELS = {fmt: _get_num_channels(fmt) for fmt in _ALL_VIDEO_FORMATS}


def get_num_channels(fmt: GstVideo.VideoFormat):
    return _ALL_VIDEO_FORMAT_CHANNELS[fmt]


def gst_video_format_from_string(frmt: str) -> GstVideo.VideoFormat:
    return GstVideo.VideoFormat.from_string(frmt)


class NumpyArrayEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, np.ndarray):
            return obj.tolist()
        elif isinstance(obj, np.float32):
            return float(obj)
        return json.JSONEncoder.default(self, obj)
