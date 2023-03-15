"""
    metadata sink plugin
"""
from adaflow.utils import NumpyArrayEncoder
from adaflow.metadata.flow_json_meta import flow_meta_get
from gi.repository import Gst, GObject, GstBase
import gi
import json
import os
import logging

gi.require_version('Gst', '1.0')
gi.require_version('GstBase', '1.0')
gi.require_version('GstVideo', '1.0')

class MetaDataSink(GstBase.BaseSink):

    GST_PLUGIN_NAME = 'metasink'

    __gstmetadata__ = (GST_PLUGIN_NAME,
                       "File metadata publisher",
                       "Publishes the JSON metadata to files.(mqtt/kafka to be done in the future)",
                       "JingYao")

    __gsttemplates__ = Gst.PadTemplate.new("sink",
                                           Gst.PadDirection.SINK,
                                           Gst.PadPresence.ALWAYS,
                                           Gst.Caps.new_any())

    __gproperties__ = {
        "method": (GObject.TYPE_STRING,  # type
                   "Publish method",  # nick
                   "Publishing method",  # blurb
                   "file",  # default
                   GObject.ParamFlags.READWRITE
                   # flags
                   ),

        "filepath": (GObject.TYPE_STRING,  # type
                   "FilePath",  # nick
                   "Absolute path to output file for publishing inferences",  # blurb
                   "",  # default
                   GObject.ParamFlags.READWRITE
                   # flags
                   ),

        "fileformat": (GObject.TYPE_STRING,  # type
                     "FileFormat",  # nick
                     "Structure of JSON objects in the file",  # blurb
                     "json",  # default
                     GObject.ParamFlags.READWRITE
                     # flags
                     ),

    }

    def __init__(self):
        super(MetaDataSink, self).__init__()
        self.method = "file"
        self.filepath = ""
        self.fileformat = "json"

    def do_get_property(self, prop: GObject.GParamSpec):
        if prop.name == 'method':
            return self.method
        elif prop.name == 'filepath':
            return self.filepath
        elif prop.name == 'fileformat':
            return self.fileformat
        else:
            raise AttributeError('unknown property %s' % prop.name)

    def do_set_property(self, prop: GObject.GParamSpec, value):
        if prop.name == 'method':
            self.method = value
        elif prop.name == 'filepath':
            self.filepath = value
        elif prop.name == 'fileformat':
            self.fileformat = value
        else:
            raise AttributeError('unknown property %s' % prop.name)

    def do_render(self, buffer):
        metadata = flow_meta_get(buffer)
        metadata = json.loads(metadata)
        if(self.method == "file"):
            self._write_result_yaml(self.filepath, metadata)

        return Gst.FlowReturn.OK


    def _write_result_yaml(self, json_path, res):

        if(os.path.exists(json_path)):
            with open(json_path, 'a') as f:
                if(self.fileformat == "json"):
                    json.dump(res, f, cls=NumpyArrayEncoder)
                elif(self.fileformat == "json-lines"):
                    json.dump(res, f, indent=1, cls=NumpyArrayEncoder)
                else:
                    logging.error("Unsupported fileformat, please set fileformat: json or json-lines\n")
                f.write('\n')

        else:
            with open(json_path, 'w') as f:
                if(self.fileformat == "json"):
                    json.dump(res, f, cls=NumpyArrayEncoder)
                elif(self.fileformat == "json-lines"):
                    json.dump(res, f, indent=1, cls=NumpyArrayEncoder)
                else:
                    logging.error("Unsupported fileformat, please set fileformat: json or json-lines\n")
                f.write('\n')

        return True

GObject.type_register(MetaDataSink)
__gstelementfactory__ = (MetaDataSink.GST_PLUGIN_NAME, Gst.Rank.NONE, MetaDataSink)

