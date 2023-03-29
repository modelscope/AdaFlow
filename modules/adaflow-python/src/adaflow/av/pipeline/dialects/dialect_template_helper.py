# Copyright (c) Alibaba, Inc. and its affiliates.
from typing import Dict
import re

SINK_TEMPLATES = {
    "application": "appsink name=%(name)s %(properties_string)s",
    "metadata": "flow_metadata_sink name=%(name)s location=%(location)s format=%(format)s",
    "oss_bucket": "flow_oss_upload_sink name=%(name)s location=%(location)s %(properties_string)s",
    "file": "filesink name=%(name)s location=%(location)s %(properties_string)s",
    "multifile": "multifilesink name=%(name)s location=%(location)s %(properties_string)s",
    "gst": "%(element)s %(properties_string)s",
    "fakesink": "fakesink "
}

SOURCE_TEMPLATES = {
    "application": "appsrc name=%(name)s %(properties_string)s",
    #"file": "filesrc name=%(name)s %(properties_string)s ! decodebin",
    "file": "filesrc name=%(name)s location=%(location)s %(properties_string)s ! decodebin",
    "camera": "v4l2src name=%(name)s device=%(device)s %(properties_string)s",
    "uri": "urisourcebin name=%(name)s uri=%(uri)s %(properties_string)s ! decodebin",
    "gst": "%(element)s %(properties_string)s"
}


def strip_command(command: str):
    return re.sub(r'\s+', " ", command).strip()


class GStreamerTemplateHelper:
    def __init__(self, task: Dict[str, any]) -> None:
        super().__init__()
        self._task = task

    def sink(self, name: str):
        for sink in self._task["sinks"]:
            if sink["name"] == name:
                type_name = sink["type"]
                properties_string = " ".join(["%s=%s" % (k, v) for k, v in sink.get("properties", {}).items()]).strip()
                variables = dict(sink)
                variables["properties_string"] = properties_string
                if type_name in SINK_TEMPLATES:
                    return strip_command(SINK_TEMPLATES[type_name] % variables)
                else:
                    raise ValueError("sink type %s not supported" % type_name)
        raise ValueError("no sink named %s is found in task definition" % name)

    def source(self, name: str):
        for source in self._task["sources"]:
            if source["name"] == name:
                type_name = source["type"]
                properties_string = " ".join(
                    ["%s=%s" % (k, v) for k, v in source.get("properties", {}).items()]).strip()
                variables = dict(source)
                variables["properties_string"] = properties_string
                if type_name in SOURCE_TEMPLATES:
                    output = SOURCE_TEMPLATES[type_name] % variables
                    if "filter" in source:
                        output += " ! " + source["filter"] + "! videoscale ! videoconvert"
                    if "post_process" in source:
                        output += " ! " + source["post_process"]
                    return strip_command(output)
                else:
                    raise ValueError("source type %s not supported" % type_name)
        raise ValueError("no source named %s is found in task definition" % name)
