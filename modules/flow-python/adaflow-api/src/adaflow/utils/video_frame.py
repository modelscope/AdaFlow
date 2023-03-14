import numpy
import json
from absl import logging
import gi
gi.require_version('Gst', '1.0')
gi.require_version("GstVideo", "1.0")
gi.require_version('GObject', '2.0')

from gi.repository import GObject, Gst, GstVideo
from adaflow.metadata.flow_json_meta import flow_meta_add, flow_meta_get, flow_meta_remove
from adaflow.utils import gst_video_format_from_string, get_num_channels, NumpyArrayEncoder


class AVDataFrame:
    def __init__(self, buffer: Gst.Buffer, caps: Gst.Caps = None, offset=0):
        self.__buffer = buffer
        self.__offset = offset

        ##video info
        struct = caps.get_structure(0)
        self.width = struct.get_int("width").value
        self.height = struct.get_int("height").value
        video_format = gst_video_format_from_string(struct.get_value('format'))
        self.channel = get_num_channels(video_format)
        self.frame_size = self.width * self.height * self.channel

    def get_json_meta(self, meta_key) -> dict:
        get_message_str = flow_meta_get(self.__buffer)
        get_message = json.loads(get_message_str)[meta_key][0]
        return get_message

    def add_json_meta(self, message, meta_key):
        get_message_str = flow_meta_get(self.__buffer)

        #first-to-add-metadata
        if get_message_str == "NULL":
            json_key_v = dict()
            json_key_v[meta_key] = []
            json_key_v[meta_key].append(message)
            json_message = json.dumps(json_key_v, cls=NumpyArrayEncoder)
            flow_meta_add(self.__buffer, json_message.encode('utf-8'))
        else:
            get_message = json.loads(get_message_str)
            if meta_key in get_message:
                logging.error(f'%s is duplicate definition, change a new key '% (meta_key))
            else:
                get_message[meta_key] = []
                get_message[meta_key].append(message)
                json_message = json.dumps(get_message, cls=NumpyArrayEncoder)
                flow_meta_remove(self.__buffer)
                flow_meta_add(self.__buffer, json_message.encode('utf-8'))

    def remove_json_meta(self):
        flow_meta_remove(self.__buffer)

    def data(self, flag=Gst.MapFlags.READ | Gst.MapFlags.WRITE) -> numpy.ndarray:
        with self.__buffer.map(flag) as info:
            image = numpy.ndarray(shape=(self.height, self.width, self.channel),dtype=numpy.uint8, buffer=info.data, offset=self.__offset)
            return image



class AVDataPacket:
    def __init__(self, buffer: Gst.Buffer, caps: Gst.Caps):
        self.__buffer = buffer
        self.__caps = caps
        #cal frame size
        struct = caps.get_structure(0)
        self.width = struct.get_int("width").value
        self.height = struct.get_int("height").value
        video_format = gst_video_format_from_string(struct.get_value('format'))
        self.channel = get_num_channels(video_format)
        self.frame_size = self.width * self.height * self.channel
        #cal buffer size
        self.buffer_size = Gst.Buffer.get_size(self.__buffer)
        #cal frame num
        self.frame_num = int(self.buffer_size/self.frame_size)

    def iterate(self):
        AVDataFrame_list =[]
        for i in range(self.frame_num):

            AVDataFrame_list.append(AVDataFrame(buffer =self.__buffer, caps = self.__caps, offset = i * self.frame_size))

        return AVDataFrame_list

    def __len__(self):
        return self.frame_num



