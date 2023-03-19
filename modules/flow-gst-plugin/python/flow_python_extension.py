"""
    AdaFlow python plugin: flow_python_extension.
    Provides a callback to execute user-defined Python functions on every frame.
    Can be used for metadata conversion, inference post-processing, and other tasks.
"""
from adaflow.utils.video_frame import AVDataPacket
from gi.repository import Gst, GObject, GstBase
import imp
import yaml
import gi

gi.require_version('Gst', '1.0')
gi.require_version('GstBase', '1.0')
gi.require_version('GstVideo', '1.0')

FORMATS = "{RGB, RGBA, I420, NV12, NV21}"

class FlowMassModelPostprocess(GstBase.BaseTransform):

    GST_PLUGIN_NAME = 'flow_python_extension'

    __gstmetadata__ = (GST_PLUGIN_NAME,
                       "extension route plugin",
                       "execute user-defined Python functions on every frame",
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

        "input": (GObject.TYPE_STRING,  # type
                                  "input",  # nick
                                  "input file or file name",  # blurb
                                  "",  # default
                                  GObject.ParamFlags.READWRITE  # flags
                                  ),
        "output": (GObject.TYPE_STRING,  # type
                                   "output",  # nick
                                   "out file or file name",  # blurb
                                   "",  # default
                                   GObject.ParamFlags.READWRITE  # flags
                                   ),
        "module": (GObject.TYPE_STRING,  # type
                                    "Python module name",  # nick
                                    "Python module name",  # blurb
                                     "",  # default
                                     GObject.ParamFlags.READWRITE  # flags
                                    ),
        "class": (GObject.TYPE_STRING,  # type
                   "(optional) Python class name",  # nick
                   "Python class name",  # blurb
                   "",  # default
                   GObject.ParamFlags.READWRITE  # flags
                   ),

        "function": (GObject.TYPE_STRING,  # type
                   "Python function name",  # nick
                   "Python function name",  # blurb
                   "postprocess",  # default
                   GObject.ParamFlags.READWRITE  # flags
                   ),

    }

    def __init__(self):

        super(FlowMassModelPostprocess, self).__init__()

        self.input = None
        self.output = None
        self.maas_module = None
        self.maas_class = None
        self.maas_function = 'postprocess'
        self.maas_post_processor = None

    def do_get_property(self, prop: GObject.GParamSpec):
        if prop.name == 'input':
            return self.input

        elif prop.name == 'output':
            return self.output

        elif prop.name == 'module':
            return self.maas_module

        elif prop.name == 'class':
            return self.maas_class

        elif prop.name == 'function':
            return self.maas_function

        else:
            raise AttributeError('unknown property %s' % prop.name)

    def do_set_property(self, prop: GObject.GParamSpec, value):
        if prop.name == 'input':
            self.input = value
        elif prop.name == 'output':
            self.output = value
        elif prop.name == 'module':
            self.maas_module = value
        elif prop.name == 'class':
            self.maas_class = value
        elif prop.name == 'function':
            self.maas_function = value
        else:
            raise AttributeError('unknown property %s' % prop.name)

    def do_set_caps(self, incaps: Gst.Caps, outcaps: Gst.Caps) -> bool:
        self.caps = incaps

        return True

    def parse_input(self, yaml_path: str):
        """
        Parsing parameters of user-defined functions.
        :param yaml_path: user-defined parameter file path
        :return: yaml data
        """
        with open(yaml_path, "r") as f:
            data = yaml.load(f, Loader=yaml.FullLoader)
        return data

    def _run_maas_post_processor(self, avdatapacket, data):
        """
        A callback to execute user-defined Python functions
        :param avdatapacket: construct VideoFrame
        :param data: yaml data
        :return:True
        """

        function = imp.load_source('flow', self.maas_module)
        import flow

        if self.maas_class is not None:
            class_name = getattr(flow, self.maas_class)
            class_name_re = class_name()
            func_name = getattr(class_name_re, self.maas_function)
            self.maas_post_processor = func_name(avdatapacket, data)

        else:
            func_name = getattr(flow, self.maas_function)
            self.maas_post_processor = func_name(avdatapacket, data)

        return True

    def do_transform_ip(self, buffer: Gst.Buffer) -> Gst.FlowReturn:
        try:
            avdatapacket = AVDataPacket(buffer, caps= self.caps).iterate()
            if self.input is not None:
                data = self.parse_input(self.input)
            else:
                data =[]

            self._run_maas_post_processor(avdatapacket, data)

            return Gst.FlowReturn.OK

        except Gst.MapError as e:
            Gst.error("Mapping error: %s" % e)
            return Gst.FlowReturn.ERROR

GObject.type_register(FlowMassModelPostprocess)
__gstelementfactory__ = (FlowMassModelPostprocess.GST_PLUGIN_NAME,
                         Gst.Rank.NONE, FlowMassModelPostprocess)
