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
        with PipelineComposer() as c:
            # update metadata
            c.maintainer("long.qul", "long.qul@alibaba-inc.com")
            c.description("fire in the hole")

            # route one
            c.node("a") >> c.node("b") >> c.node("d") >> c.node("e") \
            >> c.trunk("tee1").kind("tee") >> c.node("f") >> c.node("g") \
                    >> c.node("l")

            with c.branch_of("tee1", "branch1") as branch1:
                # route two
                branch1 >> c.node("h") >> c.node("i") >> c.node("l")

            with c.branch_of("tee1", "branch2") as branch2:
                # route three
                branch2 >> c.node("j") >> c.node("k")

            print(c.get_nodes())
            print(c.get_edges())
            # display
            c.visualize()


if __name__ == '__main__':
    unittest.main()
