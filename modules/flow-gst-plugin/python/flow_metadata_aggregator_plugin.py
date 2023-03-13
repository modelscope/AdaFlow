import gi
from PIL import Image
import numpy as np
import json

gi.require_version('Gst', '1.0')
gi.require_version('GstBase', '1.0')
gi.require_version('GObject', '2.0')

from gi.repository import Gst, GObject, GstBase, GLib
from adaflow.metadata.flow_json_meta import flow_meta_add, flow_meta_get, flow_meta_remove
from adaflow.utils import gst_video_format_from_string, get_num_channels, NumpyArrayEncoder

class BlendData:
    def __init__(self,outimg):
        self.outimg = outimg
        self.pts = 0
        self.eos = True

class GstMetaAgg(GstBase.Aggregator):

    SINK_CAPS = 'video/x-raw,format=RGB,width=[1,{max_int}],height=[1,{max_int}]'
    SINK_CAPS = Gst.Caps.from_string(SINK_CAPS.format(max_int=GLib.MAXINT))
    SRC_CAPS = 'video/x-raw,format=RGB,width=[1,{max_int}],height=[1,{max_int}]'
    SRC_CAPS = Gst.Caps.from_string(SRC_CAPS.format(max_int=GLib.MAXINT))

    __gstmetadata__ = ('meta_aggregator',
                       'plugin to aggregator more buffer metadata',
                       'Python metadata mixer',
                       'JingYao')

    __gsttemplates__ = (
        Gst.PadTemplate.new_with_gtype("sink_%u",
                                       Gst.PadDirection.SINK,
                                       Gst.PadPresence.REQUEST,
                                       SINK_CAPS,
                                       GstBase.AggregatorPad.__gtype__),
        Gst.PadTemplate.new_with_gtype("src",
                                       Gst.PadDirection.SRC,
                                       Gst.PadPresence.ALWAYS,
                                       SRC_CAPS,
                                       GstBase.AggregatorPad.__gtype__)
    )

    def __init__(self):

        super(GstMetaAgg, self).__init__()

        self.vid_pad = None
        self.vid_caps = None
        self.width = 720
        self.height = 360

    def ensure_pads_found(self):
        if self.vid_pad and self.inf_pad:
            return
        for pad in self.sinkpads:
            caps = pad.get_current_caps()
            feature = caps.get_features(0).get_nth(0)
            struct = caps.get_structure(0)

            self.width = struct.get_int("width").value
            self.height = struct.get_int("height").value
            video_format = gst_video_format_from_string(struct.get_value('format'))
            self.channel = get_num_channels(video_format)
            self.vid_pad = pad
            self.vid_caps = caps

        return True

    def do_fixate_src_caps (self, caps):
        self.ensure_pads_found()
        return self.vid_caps

    def mix_buffers(self, agg, pad, bdata):

        if(GstBase.AggregatorPad.has_buffer(pad)):
            buf = pad.pop_buffer()
            detection_info = flow_meta_get(buf)
            self.metadata.append(detection_info)
            with buf.map(Gst.MapFlags.READ) as info:
                img = np.ndarray(shape=(self.height, self.width, self.channel), dtype=np.uint8, buffer=info.data)
            bdata.outimg = img
            bdata.pts = buf.pts
            bdata.eos = False

        return True

    def do_aggregate(self, timeout):
        outimg = Image.new('RGB', (self.width, self.height), (0, 0, 0))
        bdata = BlendData(outimg)
        self.metadata = []
        self.foreach_sink_pad(self.mix_buffers, bdata)

        data = bdata.outimg.tobytes()

        outbuf = Gst.Buffer.new_allocate(None, len(data), None)
        outbuf.fill(0, data)
        outbuf.pts = bdata.pts

        if(self.metadata):
            json_key_v = dict()
            for i in range(len(self.metadata)):
                get_message = json.loads(self.metadata[i])
                key = list(get_message.keys())[0]
                json_key_v[key] = []
                json_key_v[key].append(get_message[key][0])

            json_message = json.dumps(json_key_v, cls=NumpyArrayEncoder)
            flow_meta_add(outbuf, json_message.encode('utf-8'))

            self.finish_buffer(outbuf)

        # We are EOS when no pad was ready to be aggregated,
        # this would obviously not work for live
        if bdata.eos:
            return Gst.FlowReturn.EOS

        return Gst.FlowReturn.OK


GObject.type_register(GstMetaAgg)
__gstelementfactory__ = ("meta_aggregator", Gst.Rank.NONE, GstMetaAgg)







