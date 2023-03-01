class Attribute:

    def __init__(self, name: str) -> None:
        self.name = name

    def get_name(self):
        return self.name


class EnvAttribute(Attribute):

    def __init__(self, name: str, env_name: str) -> None:
        super().__init__(name)
        self.env_name = env_name

    def get_env_name(self):
        return self.env_name


class ParameterSourceAttribute(Attribute):

    def __init__(self, name: str, json_path: str) -> None:
        super().__init__(name)
        self.json_path = json_path

    def get_json_path(self) -> str:
        return self.json_path
