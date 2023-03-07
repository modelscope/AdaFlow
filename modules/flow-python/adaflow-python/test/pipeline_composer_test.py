import logging
import unittest

from adaflow.av.pipeline import PipelineComposer
import networkx as nx
import matplotlib.pyplot as plt

class PipelineComposerTest(unittest.TestCase):

    def test_graph_visualize(self):
        DG = nx.DiGraph()
        DG.add_node("a")
        DG.add_node("b")
        nx.draw_networkx(DG)
        plt.draw()
        plt.show()

    def test_composing(self):
        c = PipelineComposer("test-piepline")
        # update metadata
        c.maintainer("long.qul", "long.qul@alibaba-inc.com")
        c.description("fire in the hole")

        # route one
        c.node("a").kind("src") >> c.node("b").kind("decode") >> c.node("d").kind("convert") >> c.node("e").kind("cspfilter") \
        >> c.trunk("tee1").kind("tee") >> c.node("f").kind("model").attr("task_id", "segmentation").attr("model_id", "xxx") \
        >> c.node("g").kind("python_extension") \
                >> c.node("l").kind("videosink")

        with c.branch_of("tee1", "branch1") as branch1:
            # route two
            branch1 >> c.node("h").kind("model") >> c.node("i").kind("python_extension") >> c.node("l")

        with c.branch_of("tee1", "branch2") as branch2:
            # route three
            branch2 >> c.node("j").kind("model") >> c.node("k")

        print(c.get_nodes())
        # print(c.get_edges())
        # for n in c._root_graph.neighbors("b"):
        #     print("node=" + n)
        #     logging.info("suc=%s" % n)
        print(c.dump_json())
        # display
        # c.visualize()


if __name__ == '__main__':
    unittest.main()
