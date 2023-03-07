from ctypes import *
import gi
import platform
import os
gi.require_version('GstVideo', '1.0')
gi.require_version('GstAudio', '1.0')
gi.require_version('GLib', '2.0')
gi.require_version('Gst', '1.0')

sys_platform = platform.platform().lower()
if "macos" in sys_platform:
    libgst = CDLL(os.getenv("LIB_GSTREAMER_PATH", "libflowmetadata.dylib"))
elif "linux" in sys_platform:
    libgst = CDLL(os.getenv("LIB_GSTREAMER_PATH", "libflowmetadata.so"))
else:
    print("other platform")

class GVAJSONMeta(Structure):
    _fields_ = [('_meta_flags', c_int),
                ('_info', c_void_p),
                ('_message', c_char_p)]

GVAJSONMetaPtr = POINTER(GVAJSONMeta)

libgst.gst_buffer_add_json_info_meta.argtypes = [c_void_p, c_char_p]
libgst.gst_buffer_add_json_info_meta.restype = c_void_p

libgst.gst_buffer_get_json_info_meta.argtypes = [c_void_p]
libgst.gst_buffer_get_json_info_meta.restype = c_char_p

libgst.gst_buffer_remove_json_info_meta.argtypes = [c_void_p]
libgst.gst_buffer_remove_json_info_meta.restype = c_bool

def flow_meta_add(buffer, message):
     _ = libgst.gst_buffer_add_json_info_meta(hash(buffer), message)

def flow_meta_get(buffer):
    res = libgst.gst_buffer_get_json_info_meta(hash(buffer))
    return res.decode('utf-8')

def flow_meta_remove(buffer):
    libgst.gst_buffer_remove_json_info_meta(hash(buffer))


