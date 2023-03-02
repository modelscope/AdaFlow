from enum import Enum


class AttributeValueType(Enum):
    INTEGER = 1
    STRING = 2
    NUMBER = 3
    BOOL = 4

class Attribute:

    def __init__(self, name: str, kind: AttributeValueType) -> None:
        self.name = name
        self.kind = type

    def get_name(self):
        return self.name

    def get_kind(self):
        return self.kind


class EnvAttribute(Attribute):

    def __init__(self, name: str, env_name: str, kind: AttributeValueType) -> None:
        super().__init__(name, kind)
        self.env_name = env_name

    def get_env_name(self):
        return self.env_name


class ParameterSourceAttribute(Attribute):

    def __init__(self, name: str, json_path: str, kind: AttributeValueType) -> None:
        super().__init__(name, kind)
        self.json_path = json_path

    def get_json_path(self) -> str:
        return self.json_path

class CommandLineArgumentAttribute(Attribute):

    def __init__(self, name: str, kind: AttributeValueType) -> None:
        super().__init__(name, kind)

