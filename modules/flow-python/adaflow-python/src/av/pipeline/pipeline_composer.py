from typing import Dict, TypeVar
from graph import Graph
import networkx as nx

PipelineComposerType = TypeVar("PipelineComposerType", bound="PipelineComposer")

class PipelineComposer:

    def __init__(self) -> None:
        super().__init__()
        self.root_graph = nx.DiGraph(name="root")

    def node(self, name: str, kind: str, attributes):
        self.root_graph.add_node()


