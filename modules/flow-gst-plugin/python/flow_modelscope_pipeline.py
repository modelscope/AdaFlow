"""
    AdaFlow python plugin: flow_modelscope_pipeline.
    Run modelscope pipeline and produce result data.
"""
from modelscope.pipelines import pipeline
from adaflow.av.utils import gst_video_format_from_string, get_num_channels,NumpyArrayEncoder
from adaflow.av.metadata.flow_json_meta import flow_meta_add_key
from gi.repository import Gst, GObject, GstBase
from absl import logging
import numpy as np

import gi

gi.require_version('Gst', '1.0')
gi.require_version('GstBase', '1.0')
gi.require_version('GstVideo', '1.0')


FORMATS = "{RGB, RGBA, I420, NV12, NV21}"

class FlowMassModelPlugin(GstBase.BaseTransform):

    GST_PLUGIN_NAME = 'flow_modelscope_pipeline'

    __gstmetadata__ = (GST_PLUGIN_NAME,
                       "almighty plugin for maas model pipeline",
                       "run modelscope pipeline and produce result data",
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
                       "task": (GObject.TYPE_STRING,  # type
                                            "model task",  # nick
                                            "maas tasks",  # blurb
                                            "",  # default
                                            GObject.ParamFlags.READWRITE
                                            # flags
                                            ),
                       "id": (GObject.TYPE_STRING,  # type
                                          "model id on the maas",  # nick
                                          "model id",  # blurb
                                          "",  # default
                                          GObject.ParamFlags.READWRITE  # flags
                                          ),
                       "input": (GObject.TYPE_STRING,  # type
                                  "image or video input path",  # nick
                                  "input path",  # blurb
                                  "",  # default
                                  GObject.ParamFlags.READWRITE  # flags
                                  ),

                       "meta-key": (GObject.TYPE_STRING,  # type
                                 "the keyword of the results which attach to the GstBuffer",  # nick
                                 "the keyword of the results",  # blurb
                                 "modelout",  # default
                                 GObject.ParamFlags.READWRITE  # flags
                                 ),
    }

    def __init__(self):

        super(FlowMassModelPlugin, self).__init__()

        self.task = None
        self.id = None
        self.input = None
        self.meta_key = 'modelout'


    def do_get_property(self, prop: GObject.GParamSpec):
        if prop.name == 'task':
            return self.task
        elif prop.name == 'id':
            return self.id
        elif prop.name == 'input':
            return self.input
        elif prop.name == 'meta-key':
            return self.meta_key
        else:
            raise AttributeError('unknown property %s' % prop.name)

    def do_set_property(self, prop: GObject.GParamSpec, value):
        if prop.name == 'task':
            self.task = value
        elif prop.name == 'id':
            self.id = value
        elif prop.name == 'input':
            self.input = value
        elif prop.name == 'meta-key':
            self.meta_key = value
        else:
            raise AttributeError('unknown property %s' % prop.name)

    def do_set_caps(self, incaps: Gst.Caps, outcaps: Gst.Caps) -> bool:
        struct = incaps.get_structure(0)
        self.width = struct.get_int("width").value
        self.height = struct.get_int("height").value
        video_format = gst_video_format_from_string(struct.get_value('format'))
        self.channel = get_num_channels(video_format)

        if self.task is None or self.id is None:
            raise ValueError(f'id = {self.id} or task = {self.task} is error ')

        #build maas pipeline
        self.maas_pipeline = pipeline(self.task, model=self.id)

        return True

    def do_transform_ip(self, buffer: Gst.Buffer) -> Gst.FlowReturn:
        try:
            with buffer.map(Gst.MapFlags.READ | Gst.MapFlags.WRITE) as info:
                image = np.ndarray(
                    shape=(self.height, self.width, self.channel),
                    dtype=np.uint8, buffer=info.data)
                if self.input is not None:
                    det_res = self.maas_pipeline(self.input)
                    logging.debug(f'{det_res}')
                else:
                    det_res = self.maas_pipeline(image)

                flow_meta_add_key(buffer, det_res, self.meta_key)

                return Gst.FlowReturn.OK
        except Gst.MapError as e:
            Gst.error("Mapping error: %s" % e)
            return Gst.FlowReturn.ERROR


GObject.type_register(FlowMassModelPlugin)
__gstelementfactory__ = (FlowMassModelPlugin.GST_PLUGIN_NAME,
                         Gst.Rank.NONE, FlowMassModelPlugin)
