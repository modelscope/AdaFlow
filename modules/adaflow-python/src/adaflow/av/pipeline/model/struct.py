# Copyright (c) Alibaba, Inc. and its affiliates.
from types import SimpleNamespace
import json

from typing import Dict


class Struct(SimpleNamespace):
    @staticmethod
    def from_json(file):
        return json.load(file, object_hook=lambda x: Struct(**x))

    @staticmethod
    def from_dict(data: Dict[str, any] = None):
        return json.loads(json.dumps(data), object_hook=lambda x: Struct(**x))

    def to_dict(self):
        return json.loads(json.dumps(self, default=lambda s: vars(s)))
