# Copyright (c) Alibaba, Inc. and its affiliates.
import logging
import numpy
import json
import numpy as np

from adaflow.av.utils import gst_video_format_from_string, get_num_channels, NumpyArrayEncoder
from adaflow.av.metadata.flow_json_meta import flow_meta_add, flow_meta_get, flow_meta_remove
import gi
from gi.repository import Gst

gi.require_version('Gst', '1.0')


class AVDataFrame:
    def __init__(self, buffer: Gst.Buffer, caps: Gst.Caps = None, offset=0):
        """
        Construct AVDataFrame instance from Gst.Buffer and Gst.Caps.
        :param buffer:Gst.Buffer to which metadata is attached and retrieved.
        :param caps:Gst.Caps from which video information is obtained.
        :param offset:Frame offset to get from buffer.
        """
        self.__buffer = buffer
        self.__offset = offset

        # video info
        struct = caps.get_structure(0)
        self.width = struct.get_int("width").value
        self.height = struct.get_int("height").value
        video_format = gst_video_format_from_string(struct.get_value('format'))
        self.channel = get_num_channels(video_format)
        self.frame_size = self.width * self.height * self.channel
        self._log = logging.Logger("AVDataFrame")

    @property
    def log(self):
        return self._log

    @property
    def buffer(self) -> Gst.Buffer:
        return self.__buffer

    def get_json_meta(self, meta_key):
        """
        Get messages attached to this VideoFrame by meta_key.
        :param meta_key:metadata keyword
        :return:json mesage
        """
        get_message_str = flow_meta_get(self.__buffer)
        if get_message_str == "NULL":
            self.log.error('AVDataFrame has not %s metadata' % meta_key)
            return "NULL"

        else:
            get_message_json = json.loads(get_message_str)
            if meta_key not in get_message_json:
                self.log.error('AVDataFrame has not %s metadata' % meta_key)

            get_message = get_message_json[meta_key][0]

        return get_message

    def add_json_meta(self, message, meta_key):
        """
        Attach message to this VideoFrame by meta_key.
        :param message:json metadata message
        :param meta_key:metadata keyword
        :return:None
        """
        get_message_str = flow_meta_get(self.__buffer)

        # first-to-add-metadata
        if get_message_str == "NULL":
            json_key_v = dict()
            json_key_v[meta_key] = []
            json_key_v[meta_key].append(message)
            json_message = json.dumps(json_key_v, cls=NumpyArrayEncoder)
            flow_meta_add(self.__buffer, json_message.encode('utf-8'))
        else:
            get_message = json.loads(get_message_str)
            if meta_key in get_message:
                self.log.error('%s is duplicate definition, change a new key ' % meta_key)
            else:
                get_message[meta_key] = []
                get_message[meta_key].append(message)
                json_message = json.dumps(get_message, cls=NumpyArrayEncoder)
                flow_meta_remove(self.__buffer)
                flow_meta_add(self.__buffer, json_message.encode('utf-8'))

    def remove_json_meta(self):
        """
        Remove message from this VideoFrame
        :return:None
        """
        flow_meta_remove(self.__buffer)

    def data(self, flag=Gst.MapFlags.READ | Gst.MapFlags.WRITE) -> np.ndarray:
        """
        Get buffer data wrapped by numpy.ndarray.
        :param flag:
        :return:numpy array instance
        """
        with self.__buffer.map(flag) as info:
            image = numpy.ndarray(
                shape=(self.height, self.width, self.channel),
                dtype=numpy.uint8,
                buffer=info.data,
                offset=self.__offset
            )
            return image
