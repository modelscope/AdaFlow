# Copyright (c) Alibaba, Inc. and its affiliates.
from __future__ import annotations

import networkx as nx
from typing import Dict, TypeVar
from .attribute import Attribute, AttributeValueType, EnvAttribute, ParameterSourceAttribute, StaticAttribute

NodeType = TypeVar("NodeType", bound="Node")
BranchNodeType = TypeVar("BranchNodeType", bound="BranchNode")
TrunkNodeType = TypeVar("TrunkNodeType", bound="TrunkNode")


class Node:

    def __init__(self, name: str, graph: nx.DiGraph, **kwargs) -> None:
        super().__init__()
        self._name = name
        self._kind = None
        self._attributes = []
        self._parent_graph = graph
        graph.add_node(name, **kwargs)

    def get_name(self) -> str:
        return self._name

    def name(self, name: str) -> NodeType:
        self._name = name
        return self

    def get_attributes(self) -> [EnvAttribute|ParameterSourceAttribute]:
        return self._attributes

    def get_kind(self) -> str:
        return self._kind

    def kind(self, kind: str) -> NodeType:
        self._kind = kind
        return self

    def __rshift__(self, other: NodeType) -> NodeType:
        self._parent_graph.add_edge(self.get_name(), other.get_name())
        return other

    def __lshift__(self, other: NodeType) -> NodeType:
        self._parent_graph.add_edge(other.get_name(), self.get_name())
        return self

    def env_attr(self, name: str, env_name: str, kind: AttributeValueType = AttributeValueType.STRING) -> NodeType:
        self._attributes.append(EnvAttribute(name, env_name, kind))
        return self

    def parameter_source_attr(self, name: str, json_path: str, kind: AttributeValueType) -> NodeType:
        self._attributes.append(ParameterSourceAttribute(name, json_path, kind))
        return self

    def attr(self, name: str, value: any, kind: AttributeValueType = AttributeValueType.STRING) -> NodeType:
        self._attributes.append(StaticAttribute(name, kind, value))
        return self

    def dump_object(self):
        return {
            "kind": self.get_kind(),
            "name": self.get_name(),
            "attributes": [attr.dump_object() for attr in self.get_attributes()],
            "successors":  [str(successor) for successor in self._parent_graph.neighbors(self._name)]
        }


class TrunkNode(Node):

    def __init__(self, name: str, graph: nx.DiGraph, **kwargs) -> None:
        super().__init__(name, graph, **kwargs)


class BranchNode(Node):
    def __init__(self, name: str, graph: nx.DiGraph, **kwargs) -> None:
        super().__init__(name, graph, **kwargs)
        self._trunk_name = None

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        return self

    def of(self, trunk_name: str) -> BranchNodeType:
        self._trunk_name = trunk_name
        return self

    def get_truck_name(self) -> str:
        return self._trunk_name

