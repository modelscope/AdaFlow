"""
    AdaFlow python plugin: flow_modelscope_pipeline.
    Run modelscope pipeline and produce result data.
"""
from modelscope.pipelines import pipeline
from modelscope.outputs import OutputKeys
from adaflow.av.utils import gst_video_format_from_string, get_num_channels
from adaflow.av.metadata.flow_json_meta import flow_meta_add_key
from gi.repository import Gst, GObject, GstVideo
import logging
import numpy as np

import gi

gi.require_version('Gst', '1.0')
gi.require_version('GstVideo', '1.0')


class FlowMassModelPlugin(Gst.Element):
    GST_PLUGIN_NAME = 'flow_modelscope_pipeline'

    __gstmetadata__ = (GST_PLUGIN_NAME,
                       "almighty plugin for maas model pipeline",
                       "run modelscope pipeline and produce result data",
                       "JingYao")

    __gsttemplates__ = (Gst.PadTemplate.new("src",
                                            Gst.PadDirection.SRC,
                                            Gst.PadPresence.ALWAYS,
                                            Gst.Caps.new_any()),
                        Gst.PadTemplate.new("sink",
                                            Gst.PadDirection.SINK,
                                            Gst.PadPresence.ALWAYS,
                                            Gst.Caps.new_any()))

    _sinkpadtemplate = __gsttemplates__[1]
    _srcpadtemplate = __gsttemplates__[0]

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

        "add-meta": (GObject.TYPE_BOOLEAN,  # type
                     "whether to add metadata at buffer",  # nick
                     "whether to add metadata at buffer",  # blurb
                     True,  # default
                     GObject.ParamFlags.READWRITE  # flags
                     ),
    }

    def __init__(self):

        super(FlowMassModelPlugin, self).__init__()

        self.task = None
        self.id = None
        self.input = None
        self.add_meta = True
        self.meta_key = 'modelout'

        self.sinkpad = Gst.Pad.new_from_template(self._sinkpadtemplate, 'sink')
        self.sinkpad.set_chain_function_full(self.chainfunc, None)
        self.sinkpad.set_event_function_full(self.eventfunc, None)

        self.add_pad(self.sinkpad)
        self.srcpad = Gst.Pad.new_from_template(self._srcpadtemplate, 'src')
        self.add_pad(self.srcpad)
        self.maas_pipeline = None

    def do_get_property(self, prop: GObject.GParamSpec):

        if prop.name == 'task':
            return self.task
        elif prop.name == 'id':
            return self.id
        elif prop.name == 'input':
            return self.input
        elif prop.name == 'meta-key':
            return self.meta_key
        elif prop.name == 'add-meta':
            return self.add_meta
        else:
            raise AttributeError('unknown property %s' % prop.name)

    def do_set_property(self, prop: GObject.GParamSpec, value):
        if prop.name == 'task':
            self.task = value
        elif prop.name == 'id':
            self.id = value
            # init model
            if self.id and self.task:
                self.maas_pipeline = pipeline(self.task, model=self.id)

        elif prop.name == 'input':
            self.input = value
        elif prop.name == 'meta-key':
            self.meta_key = value
        elif prop.name == 'add-meta':
            self.add_meta = value
        else:
            raise AttributeError('unknown property %s' % prop.name)

    def eventfunc(self, pad, parent, event):
        # check pars
        if self.task is None or self.id is None:
            raise ValueError(f'id = {self.id} or task = {self.task} is error ')

        return self.srcpad.push_event(event)

    def chainfunc(self, pad: Gst.Pad, parent, buffer: Gst.Buffer) -> Gst.FlowReturn:
        # buffer-in-info
        incaps = self.sinkpad.get_current_caps()
        struct = incaps.get_structure(0)
        width_in = struct.get_int("width").value
        height_in = struct.get_int("height").value
        video_format = gst_video_format_from_string(struct.get_value('format'))
        channel_in = get_num_channels(video_format)
        # make buffer writable after tee
        buffer = Gst.Buffer.copy_deep(buffer)

        with buffer.map(Gst.MapFlags.READ | Gst.MapFlags.WRITE) as info:

            image = np.ndarray(shape=(height_in, width_in, channel_in), dtype=np.uint8, buffer=info.data)

            if self.input is not None:
                infer_res = self.maas_pipeline(self.input)
                logging.debug(f'{infer_res}')
            else:
                infer_res = self.maas_pipeline(image)

            if self.add_meta:
                flow_meta_add_key(buffer, infer_res, self.meta_key)
                return self.srcpad.push(buffer)
            else:
                infer_image = infer_res[OutputKeys.OUTPUT_IMG]
                # set new outcaps
                height_out, width_out, channel_out = infer_image.shape
                if height_out != height_in or width_out != width_in:
                    outcaps = self.srcpad.get_current_caps()
                    videoinfo = GstVideo.videoinfo.new_from_caps(outcaps)
                    videoinfo.width = width_out
                    videoinfo.height = height_out
                    outcaps = GstVideo.videoinfo.to_caps(videoinfo)
                    self.srcpad.set_caps(outcaps)

                infer_data = infer_image.tobytes()
                newbuf = Gst.Buffer.new_allocate(None, len(infer_data), None)
                newbuf.fill(0, infer_data)
                return self.srcpad.push(newbuf)


GObject.type_register(FlowMassModelPlugin)
__gstelementfactory__ = (FlowMassModelPlugin.GST_PLUGIN_NAME,
                         Gst.Rank.NONE, FlowMassModelPlugin)
