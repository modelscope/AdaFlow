from ..model.struct import Struct

SINK_TEMPLATES = {
    "application": "appsink name=%(name)s $(properties_string)s",
    "notify": "flow_metadata_sink name=%(name)s output=%{output}s format=%{format}s %{properties_string}s",
    "oss": "flow_oss_upload_sink name=%(name)s output=%{output}s",
    "file": "multifilesink name=%(name)s location=%(location)s  %{properties_string}s",
    "gst": "$(element)s $(properties_string)s"
}

SOURCE_TEMPLATES = {
    "application": "appsrc name=%(name)s $(properties_string)s",
    "file": "urisourcebin name=%(name)s uri=%(uri)s %(properties_string)s ! decodebin ! videoconvert",
    "rtsp": "urisourcebin name=%(name)s uri=%(uri)s %(properties_string)s ! decodebin ! videoconvert",
    "camera": "v4l2src name=%(name)s device=%(device)s %(properties_string)s ! videoconvert",
    "uri": "urisourcebin name=%(name)s uri=%(uri)s %(properties_string)s ! videoconvert",
    "gst": "%(element)s %(properties_string)s"
}


class GStreamerTemplateHelper:
    def __init__(self, task: Struct) -> None:
        super().__init__()
        self._task = task

    def sink(self, name: str):
        for k, sink in self._task.sinks:
            if sink["name"] == name:
                type_name = sink["type"]
                properties_string = ["%s=%s" % (k, v) for k, v in sink.get("properties", {})]
                variables = dict(sink)
                variables["properties_string"] = properties_string
                if type_name in SINK_TEMPLATES:
                    return SINK_TEMPLATES[type_name] % variables
                else:
                    raise ValueError("sink type %s not supported" % type_name)
        raise ValueError("no sink named %s is found in task definition" % name)

    def source(self, name: str):
        for k, source in self._task.sources:
            if source["name"] == name:
                type_name = source["type"]
                properties_string = ["%s=%s" % (k, v) for k, v in source.get("properties", {})]
                variables = dict(source)
                variables["properties_string"] = properties_string
                if type_name in SOURCE_TEMPLATES:
                    output = SOURCE_TEMPLATES[type_name] % variables
                    if source["filter"]:
                        output += " ! " + source["filter"]
                    if source["post_process"]:
                        output += " ! " + source["post_process"]
                    return output
                else:
                    raise ValueError("source type %s not supported" % type_name)
        raise ValueError("no source named %s is found in task definition" % name)

