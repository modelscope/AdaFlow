from typing import Dict, TypeVar
import networkx as nx
from .attribute import Attribute
from .graph import Node, TrunkNode, BranchNode
from enum import Enum


PipelineComposerType = TypeVar("PipelineComposerType", bound="PipelineComposer")

class BackendType(Enum):
    GSTREMAER = 1


class PipelineComposer:

    @staticmethod
    def from_json_str(json_str: str) -> PipelineComposerType:
        pass

    def __init__(self):
        super().__init__()
        self._root_graph = nx.DiGraph(name="root")
        self._nodes = {}
        self._maintainers = []
        self._desc = ""
        self._backend = BackendType.GSTREMAER
        self._schema_version = 1

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        return self

    def get_nodes(self):
        return list(self._root_graph.nodes)

    def get_edges(self):
        return list(self._root_graph.edges)

    def maintainer(self, name: str, email: str) -> PipelineComposerType:
        self._maintainers.append({"name": name, "email": email})
        return self

    def description(self, desc: str) -> PipelineComposerType:
        self._desc = desc
        return self

    def schema_version(self, version: int) -> PipelineComposerType:
        self._schema_version = version
        return self

    def gstreamer_backend(self) -> PipelineComposerType:
        self._backend = BackendType.GSTREMAER
        return self

    def node(self, name: str) -> Node:
        if name in self._nodes:
            # return node with same node
            return self._nodes[name]
        node = Node(name, self._root_graph)
        self._nodes[name] = node
        return node

    def trunk(self, name: str) -> TrunkNode:
        if name in self._nodes:
            # return node with same node
            return self._nodes[name]
        node = TrunkNode(name, self._root_graph)
        self._nodes[name] = node
        return node

    def branch(self, name: str) -> BranchNode:
        if name in self._nodes:
            # return node with same node
            return self._nodes[name]
        node = BranchNode(name, self._root_graph)
        self._nodes[name] = node
        return node

    def dump_json(self) -> str:
        pass

    def visualize(self):
        import matplotlib.pyplot as plt
        nx.draw_networkx(self._root_graph, **{
            "font_size": 10,
            "node_size": 3000,
            "node_color": "white",
            "edgecolors": "black",
            "linewidths": 2,
            "width": 5,
        })
        plt.draw()
        plt.show()
