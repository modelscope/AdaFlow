import networkx as nx
from typing import Dict, TypeVar
from attribute import Attribute

GraphType = TypeVar("GraphType", bound="Graph")
NodeType = TypeVar("NodeType", bound="Node")


class Node:

    def __init__(self, name: str, kind: str, attributes: [Attribute], graph: GraphType) -> None:
        super().__init__()
        self.name = name
        self.kind = kind
        self.attributes = attributes
        self.parent_graph = graph

    def get_parent_graph(self) -> GraphType:
        return self.parent_graph

    def get_name(self) -> str:
        return self.name

    def get_attributes(self) -> [Attribute]:
        return self.attributes

    def get_kind(self) -> str:
        return self.kind

    def __rshift__(self, other: NodeType) -> NodeType:
        self.parent_graph.edge(self, other)
        return self

    def __lshift__(self: GraphType, other: NodeType) -> GraphType:
        self.parent_graph.edge(other, self)
        return self


class Graph(Node):
    def __init__(self, **kwargs) -> None:
        """
        : param name: graph name
        : param kwargs.description: graph description
        """
        super().__init__(**kwargs)

    def all_nodes(self) -> [Node]:
        """
        Return the all nodes inside this graph
        :return:
        """
        return self.nodes

    def all_edges(self):
        pass

    def edge(self, left: Node, right: Node):
        pass

    def node(self, node: Node):
        pass
