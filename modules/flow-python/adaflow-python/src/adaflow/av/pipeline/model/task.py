from types import SimpleNamespace

from typing import Dict
import json


class Task(SimpleNamespace):
    @staticmethod
    def from_json(file):
        return json.load(file, object_hook=lambda x: Task(**x))

    @staticmethod
    def from_dict(data):
        return json.loads(json.dumps(data), object_hook=lambda x: Task(**x))
