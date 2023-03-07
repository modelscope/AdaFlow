from typing import Dict
import gi
import networkx as nx
from adaflow.av.pipeline import PipelineComposer
from .base_pipeline import BasePipeline

# gi.require_version("Gst", "1.0")
# gi.require_version("GstApp", "1.0")
# gi.require_version("GstVideo", "1.0")
# from gi.repository import Gst, GLib, GObject, GstApp, GstVideo



class GStreamerPipeline(BasePipeline):
    def __init__(self, pipeline_composer: PipelineComposer) -> None:
        super().__init__(pipeline_composer)

    def startup(self, task: Dict[str, any]):
        graph = self.composer.graph
        roots = []
        leafs = []
        for node in graph.nodes:
            if graph.in_degree(node) == 0:
                roots.append(node)
                print("root node: " +  node)
            if graph.out_degree(node) == 0:
                leafs.append(node)
                print("leaf node: " + node)

        for root in roots:

            for leaf in leafs:
                for path in nx.all_simple_paths(graph, root, leaf):
                    print(path)

    def stop(self):
        pass

    def shutdown(self):
        pass

    def push(self, src_name: str, data_packet):
        pass

    def pop(self, sink_name: str):
        pass


