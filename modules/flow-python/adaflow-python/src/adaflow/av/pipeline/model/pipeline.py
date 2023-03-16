import json
from types import SimpleNamespace

from typing import Dict, Any
from .task import Task


class Pipeline(SimpleNamespace):
    @staticmethod
    def from_json(file):
        return json.load(file, object_hook=lambda x: Pipeline(**x))

    @staticmethod
    def from_dict(data):
        return json.loads(json.dumps(data), object_hook=lambda x: Pipeline(**x))

