import networkx as nx
from typing import Dict, TypeVar
from attribute import Attribute, AttributeValueType, EnvAttribute, ParameterSourceAttribute

NodeType = TypeVar("NodeType", bound="Node")
BranchNodeType = TypeVar("BranchNodeType", bound="BranchNode")

class Node:

    def __init__(self, name: str, graph: nx.DiGraph, kind: str, attributes: [Attribute], **kwargs) -> None:
        super().__init__()
        self._name = name
        self._kind = kind
        self._attributes = attributes
        self._parent_graph = graph
        graph.add_node(self.name, **kwargs)

    def get_name(self) -> str:
        return self._name

    def name(self, name: str) -> NodeType:
        self._name = name
        return self

    def get_attributes(self) -> [Attribute]:
        return self._attributes

    def attribute(self, attr: Attribute) -> NodeType:
        self._attributes.append(attr)
        return self

    def get_kind(self) -> str:
        return self._kind

    def kind(self, kind: str) -> NodeType:
        self._kind = kind
        return self

    def __rshift__(self, other: NodeType) -> NodeType:
        self._parent_graph.add_edge(self.get_name(), other.name)
        return self

    def __lshift__(self, other: NodeType) -> NodeType:
        self._parent_graph.add_edge(other.get_name(), self)
        return self

    def env_attr(self, name: str, env_name: str, kind: AttributeValueType) -> NodeType:
        self._attributes.append(EnvAttribute(name, env_name, kind))
        return self

    def parameter_source_attr(self, name: str, json_path: str, kind: AttributeValueType) -> NodeType:
        self._attributes.append(ParameterSourceAttribute(name, json_path, kind))
        return self


class TrunkNode(Node):

    def __init__(self, name: str, graph: nx.DiGraph, kind: str, attributes: [Attribute], **kwargs) -> None:
        super().__init__(name, graph, kind, attributes, **kwargs)
        if kind:
            assert kind in ["tee", "demux"]


class BranchNode(Node):
    def __init__(self, name: str, graph: nx.DiGraph, kind: str, attributes: [Attribute], **kwargs) -> None:
        super().__init__(name, graph, kind, attributes, **kwargs)
        self._trunk_name = None

    def of(self, trunk_name: str) -> BranchNodeType:
        self._trunk_name = trunk_name
        return self

    def get_truck_name(self) -> str:
        return self._trunk_name


