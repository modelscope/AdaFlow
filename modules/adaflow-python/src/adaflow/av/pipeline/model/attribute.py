# Copyright (c) Alibaba, Inc. and its affiliates.
from enum import IntEnum


class AttributeValueType(IntEnum):
    INTEGER = 1
    STRING = 2
    NUMBER = 3
    BOOL = 4


class Attribute:

    def __init__(self, name: str, kind: AttributeValueType) -> None:
        self._name = name
        self._kind = kind

    def get_name(self):
        return self._name

    def get_kind(self):
        return self._kind


class StaticAttribute(Attribute):
    def __init__(self, name: str, kind: AttributeValueType, value: any) -> None:
        super().__init__(name, kind)
        self._value = value

    def get_value(self):
        return self._value

    def dump_object(self):
        return {
            "name": self.get_name(),
            "value_type": self.get_kind(),
            "value": self._value
        }


class EnvAttribute(Attribute):

    def __init__(self, name: str, env_name: str, kind: AttributeValueType) -> None:
        super().__init__(name, kind)
        self.env_name = env_name

    def get_env_name(self):
        return self.env_name

    def dump_object(self):
        return {
            "name": self._name,
            "value_type": self._kind,
            "kind": "env"
        }


class ParameterSourceAttribute(Attribute):

    def __init__(self, name: str, json_path: str, kind: AttributeValueType) -> None:
        super().__init__(name, kind)
        self._json_path = json_path

    def get_json_path(self) -> str:
        return self._json_path

    def dump_object(self):
        return {
            "name": self.get_name(),
            "json_path": self._json_path,
            "value_type": self.get_kind()
        }


class CommandLineArgumentAttribute(Attribute):

    def __init__(self, name: str, kind: AttributeValueType) -> None:
        super().__init__(name, kind)

