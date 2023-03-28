"""
    AdaFlow python plugin: flow_python_extension.
    Provides a callback to execute user-defined Python functions on every frame.
    Can be used for metadata conversion, inference post-processing, and other tasks.
"""

from adaflow.av.data.av_data_packet import AVDataPacket
from gi.repository import Gst, GObject, GstBase
import sys
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
                                            Gst.Caps.from_string(f"video/x-raw,format={FORMATS}")),
                        Gst.PadTemplate.new("sink",
                                            Gst.PadDirection.SINK,
                                            Gst.PadPresence.ALWAYS,
                                            # Set to RGB format
                                            Gst.Caps.from_string(f"video/x-raw,format={FORMATS}")))

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

    @staticmethod
    def parse_input(yaml_path: str):
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
        function = self.load_module_from_file('flow', self.maas_module)

        if self.maas_class is not None:
            class_name = getattr(function, self.maas_class)
            class_name_re = class_name()
            func_name = getattr(class_name_re, self.maas_function)
            self.maas_post_processor = func_name(avdatapacket, data)

        else:
            func_name = getattr(function, self.maas_function)
            self.maas_post_processor = func_name(avdatapacket, data)

        return True

    def do_transform_ip(self, buffer: Gst.Buffer) -> Gst.FlowReturn:
        try:
            avdatapacket = AVDataPacket(buffer, caps=self.caps)
            if self.input is not None:
                data = self.parse_input(self.input)
            else:
                data = []

            self._run_maas_post_processor(avdatapacket, data)

            return Gst.FlowReturn.OK

        except Gst.MapError as e:
            Gst.error("Mapping error: %s" % e)
            return Gst.FlowReturn.ERROR

    @staticmethod
    def load_module_from_file(module_name, module_path):
        """Loads a python module from the path of the corresponding file.
        Args:
            module_name (str): namespace where the python module will be loaded,
                e.g. ``foo.bar``
            module_path (str): path of the python file containing the module
        Returns:
            A valid module object
        Raises:
            ImportError: when the module can't be loaded
            FileNotFoundError: when module_path doesn't exist
        """
        if sys.version_info[0] == 3 and sys.version_info[1] >= 5:
            import importlib.util
            spec = importlib.util.spec_from_file_location(module_name, module_path)
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
        elif sys.version_info[0] == 3 and sys.version_info[1] < 5:
            import importlib.machinery
            loader = importlib.machinery.SourceFileLoader(module_name, module_path)
            module = loader.load_module()
        elif sys.version_info[0] == 2:
            import imp
            module = imp.load_source(module_name, module_path)

        return module


GObject.type_register(FlowMassModelPostprocess)
__gstelementfactory__ = (FlowMassModelPostprocess.GST_PLUGIN_NAME,
                         Gst.Rank.NONE, FlowMassModelPostprocess)
