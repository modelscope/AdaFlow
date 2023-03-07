"""
    mass model postprocess plugin ,only use route subplugin

    example:
    gst-launch-1.0 filesrc location=/xxx/image_smoke.jpg ! \
    decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
    mass_model task=domain-specific-object-detection id = damo/cv_tinynas_human-detection_damoyolo ! \
    videoconvert ! jpegenc ! filesink location=/xxx/detection_result.jpg
"""
from flow.register.constants import XSTPluginName, XSTPropertiesName
from flow.utils import gst_video_format_from_string, get_num_channels, NumpyArrayEncoder
from flow.metadata.flow_json_meta import flow_meta_add, flow_meta_get, flow_meta_remove
from gi.repository import Gst, GObject, GstBase
import numpy as np
import json
import imp
import inspect

import gi

gi.require_version('Gst', '1.0')
gi.require_version('GstBase', '1.0')
gi.require_version('GstVideo', '1.0')


FORMATS = "{RGB, RGBA, I420, NV12, NV21}"

class FlowMassModelPostprocess(GstBase.BaseTransform):

    GST_PLUGIN_NAME = XSTPluginName.MASS_MODEL_POST

    __gstmetadata__ = (GST_PLUGIN_NAME,
                       "mass model postprocess route",
                       "mass model postprocess",
                       "JingYao")

    __gsttemplates__ = (Gst.PadTemplate.new("src",
                                            Gst.PadDirection.SRC,
                                            Gst.PadPresence.ALWAYS,
                                            # Set to RGB format
                                            Gst.Caps.from_string(
                                                f"video/x-raw,format={FORMATS}")),
                        Gst.PadTemplate.new("sink",
                                            Gst.PadDirection.SINK,
                                            Gst.PadPresence.ALWAYS,
                                            # Set to RGB format
                                            Gst.Caps.from_string(
                                                f"video/x-raw,format={FORMATS}")))

    __gproperties__ = {

        XSTPropertiesName.INPUT: (GObject.TYPE_STRING,  # type
                                  "input",  # nick
                                  "input file or file name",  # blurb
                                  "",  # default
                                  GObject.ParamFlags.READWRITE  # flags
                                  ),
        XSTPropertiesName.OUTPUT: (GObject.TYPE_STRING,  # type
                                   "output",  # nick
                                   "out file or file name",  # blurb
                                   "",  # default
                                   GObject.ParamFlags.READWRITE  # flags
                                   ),
        XSTPropertiesName.MODULE: (GObject.TYPE_STRING,  # type
                                    "Python module name",  # nick
                                    "Python module name",  # blurb
                                     "",  # default
                                     GObject.ParamFlags.READWRITE  # flags
                                    ),

    }

    def __init__(self):

        super(FlowMassModelPostprocess, self).__init__()

        self.input = None
        self.output = None
        self.mass_module = None
        self.mass_post_processor = None

    def do_get_property(self, prop: GObject.GParamSpec):
        if prop.name == XSTPropertiesName.INPUT:
            return self.input

        elif prop.name == XSTPropertiesName.OUTPUT:
            return self.output

        elif prop.name == XSTPropertiesName.MODULE:
            return self.mass_module

        else:
            raise AttributeError('unknown property %s' % prop.name)

    def do_set_property(self, prop: GObject.GParamSpec, value):
        if prop.name == XSTPropertiesName.INPUT:
            self.input = value
            if self.mass_module is not None:
                self._create_mass_post_processor()
        elif prop.name == XSTPropertiesName.OUTPUT:
            self.output = value
        elif prop.name == XSTPropertiesName.MODULE:
            self.mass_module = value
            if self.input is not None:
                self._create_mass_post_processor()
        else:
            raise AttributeError('unknown property %s' % prop.name)

    def do_set_caps(self, incaps: Gst.Caps, outcaps: Gst.Caps) -> bool:
        struct = incaps.get_structure(0)
        self.width = struct.get_int("width").value
        self.height = struct.get_int("height").value
        video_format = gst_video_format_from_string(struct.get_value('format'))
        self.channel = get_num_channels(video_format)

        self.mass_post_processor.videoinfo(height=self.height, width=self.width, channel=self.channel)

        return True

    def _create_mass_post_processor(self):

        function = imp.load_source('flow', self.mass_module)
        #find class name
        class_obj = []
        for name, obj in inspect.getmembers(function):
            if inspect.isclass(obj):
                class_obj.append(obj)

        self.mass_post_processor = class_obj[1]()

        if self.input is not None:
            self.mass_post_processor.parse_input(self.input)


    def do_transform_ip(self, buffer: Gst.Buffer) -> Gst.FlowReturn:
        try:
            with buffer.map(Gst.MapFlags.READ | Gst.MapFlags.WRITE) as info:
                image = np.ndarray(
                    shape=(self.height, self.width, self.channel),
                    dtype=np.uint8, buffer=info.data)

                get_message_str = flow_meta_get(buffer)
                get_message = json.loads(get_message_str)

                outres = self.mass_post_processor.process(get_message, image)

                json_new_message = json.dumps(outres['metadata'], cls=NumpyArrayEncoder)
                flow_meta_remove(buffer)
                flow_meta_add(buffer, json_new_message.encode('utf-8'))

                return Gst.FlowReturn.OK
        except Gst.MapError as e:
            Gst.error("Mapping error: %s" % e)
            return Gst.FlowReturn.ERROR

GObject.type_register(FlowMassModelPostprocess)
__gstelementfactory__ = (FlowMassModelPostprocess.GST_PLUGIN_NAME,
                         Gst.Rank.NONE, FlowMassModelPostprocess)
