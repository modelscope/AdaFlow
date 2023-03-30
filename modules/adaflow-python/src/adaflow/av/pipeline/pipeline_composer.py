# Copyright (c) Alibaba, Inc. and its affiliates.
import json
from typing import Dict, TypeVar
import networkx as nx
from .model.attribute import Attribute
from .model.graph import Node, TrunkNode, BranchNode
from enum import Enum
import logging


PipelineComposerType = TypeVar("PipelineComposerType", bound="PipelineComposer")

class BackendType(Enum):
    GSTREMAER = 1


class PipelineComposer:

    @staticmethod
    def from_json_str(json_str: str) -> PipelineComposerType:
        pass

    def __init__(self, name: str):
        super().__init__()
        self._root_graph = nx.DiGraph(name="root")
        self._nodes = {}
        self._maintainers = []
        self._desc = ""
        self._backend = BackendType.GSTREMAER
        self._schema_version = 1
        self._name = name

    def __enter__(self):
        logging.debug("begin to compose pipeline: %s" % self._name)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if exc_val:
            logging.error("finish compose pipeline: %s" % self._name, exc_val)
        else:
            logging.info("finish compose pipeline: %s" % self._name)

        return self

    @property
    def graph(self):
        return self._root_graph

    @property
    def nodes(self):
        return self._nodes

    def get_name(self):
        return self._name

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

    def branch_of(self, trunk_name: str, branch_name: str) -> BranchNode:
        if trunk_name not in self._nodes:
            raise RuntimeError("trunk %s doesn't exist" % trunk_name)
        node = BranchNode(branch_name, self._root_graph)
        self._nodes[branch_name] = node
        self._nodes[trunk_name] >> node
        return node

    def dump_object(self) -> Dict[str, any]:
        data = {
            "name": self._name,
            "maintainer": self._maintainers,
            "description": self._desc,
            "nodes": {}
        }
        for n in self._root_graph.nodes:
            node = self._nodes[n]
            data["nodes"][node.get_name()] = node.dump_object()

        return data

    def dump_json(self) -> str:
        return json.dumps(self.dump_object(), indent=4)

    def visualize(self):
        import matplotlib.pyplot as plt
        nx.draw_networkx(self._root_graph, **{
            "node_color": "white",
            "edgecolors": "black",
        })
        plt.draw()
        plt.show()


class DialectV1PipelineComposer(PipelineComposer):

    def __init__(self):
        super().__init__()

    def adaptive_src(self):
        pass

    def file_sink(self, file_path: str):
        pass

    def curl_sink(self, uri: str, **kwargs):
        pass

    def video_sink(self):
        pass

    def audio_sink(self):
        pass

    def convert(self):
        pass

    def mux(self):
        pass

    def demux(self):
        pass

    def modelscope_task(self, task_id: str, model_id: str):
        pass

    def python_extension_metadata_merge(self):
        pass

    def python_extension(self, module:str, class_name: str, function_name: str):
        pass

    def video_aggregate(self, batch_size: int, mode: int):
        pass

    def video_split(self, batch_size: int, mode: int):
        pass
