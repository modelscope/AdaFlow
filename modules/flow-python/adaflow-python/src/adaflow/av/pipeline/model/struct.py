from types import SimpleNamespace
import json


class Struct(SimpleNamespace):
    @staticmethod
    def from_json(file):
        return json.load(file, object_hook=lambda x: Struct(**x))

    @staticmethod
    def from_dict(data):
        return json.loads(json.dumps(data), object_hook=lambda x: Struct(**x))

