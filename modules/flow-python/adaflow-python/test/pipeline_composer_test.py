import unittest

from av.pipeline.pipeline_composer import PipelineComposer


class PipelineComposerTest(unittest.TestCase):

    def test_composing(self):
        with PipelineComposer() as c:
            # update metadata
            c.maintainer("long.qul", "long.qul@alibaba-inc.com")
            c.description("fire in the hole")

            # route one
            c.node("a") >> c.node("b") >> c.node("d") >> c.node("e") \
            >> c.trunk("tee1").kind("tee") >> c.node("f") >> c.node("g") \
                    >> c.node("l")

            with c.branch("branch1").of("tee1") as branch1:
                # route two
                branch1 >> c.node("h") >> c.node("i") >> c.node("l")

            with c.branch("branch2").of("tee2") as branch2:
                # route three
                branch2 >> c.node("j") >> c.node("k")

            # display
            c.visualize()


if __name__ == '__main__':
    unittest.main()
