# Copyright (c) Alibaba, Inc. and its affiliates.
"""
JSON MetaData python wrapper
"""
from ctypes import *
import gi
import platform
import os
import json
import logging
from adaflow.av.utils import NumpyArrayEncoder

gi.require_version('GstVideo', '1.0')
gi.require_version('GstAudio', '1.0')
gi.require_version('GLib', '2.0')
gi.require_version('Gst', '1.0')

logger = logging.getLogger("JSONMetadata")
sys_platform = platform.platform().lower()

if "macos" in sys_platform or "darwin" in sys_platform:
    libgst = CDLL(os.getenv("LIB_GSTREAMER_PATH", "libflowmetadata.dylib"))
elif "linux" in sys_platform:
    libgst = CDLL(os.getenv("LIB_GSTREAMER_PATH", "libflowmetadata.so"))
else:
    print("other platform")


class FLOWJSONMeta(Structure):
    _fields_ = [('_meta_flags', c_int),
                ('_info', c_void_p),
                ('_message', c_char_p)]


FLOWJSONMetaPtr = POINTER(FLOWJSONMeta)

libgst.gst_buffer_add_json_info_meta.argtypes = [c_void_p, c_char_p]
libgst.gst_buffer_add_json_info_meta.restype = c_void_p

libgst.gst_buffer_get_json_info_meta.argtypes = [c_void_p]
libgst.gst_buffer_get_json_info_meta.restype = c_char_p

libgst.gst_buffer_remove_json_info_meta.argtypes = [c_void_p]
libgst.gst_buffer_remove_json_info_meta.restype = c_bool


def flow_meta_add(buffer, message):
    # Writes json message to Gst.Buffer
    _ = libgst.gst_buffer_add_json_info_meta(hash(buffer), message)


def flow_meta_get(buffer):
    # Gets json message to Gst.Buffer
    res = libgst.gst_buffer_get_json_info_meta(hash(buffer))
    return res.decode('utf-8')


def flow_meta_remove(buffer):
    # Removes json message to Gst.Buffer
    libgst.gst_buffer_remove_json_info_meta(hash(buffer))


def flow_meta_add_key(buffer, message, meta_key):
    # Writes json message to Gst.Buffer with meta_key
    get_message_str = flow_meta_get(buffer)

    # first-to-add-metadata
    if get_message_str == "NULL":
        json_key_v = dict()
        json_key_v[meta_key] = []
        json_key_v[meta_key].append(message)
        json_message = json.dumps(json_key_v, cls=NumpyArrayEncoder)
        flow_meta_add(buffer, json_message.encode('utf-8'))
    else:
        get_message = json.loads(get_message_str)
        if meta_key in get_message:
            logger.error('%s is duplicate definition, change a new key ' % meta_key)
        else:
            get_message[meta_key] = []
            get_message[meta_key].append(message)
            json_message = json.dumps(get_message, cls=NumpyArrayEncoder)
            flow_meta_remove(buffer)
            flow_meta_add(buffer, json_message.encode('utf-8'))
