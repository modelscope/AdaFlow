from types import SimpleNamespace

from typing import Dict


class Task(SimpleNamespace):

    def source(self, src_name: str):
        pass

    def sink(self, sink_name: str):
        pass

    def parameters(self) -> Dict[str, any]:
        pass
