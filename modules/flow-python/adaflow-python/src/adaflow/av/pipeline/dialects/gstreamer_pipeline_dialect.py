from adaflow.av.pipeline import PipelineComposer
from pipeline_dialect import PipelineDialect


class GStreamerPipelineDialect(PipelineDialect):
    def __init__(self, pipeline_composer: PipelineComposer) -> None:
        super().__init__(pipeline_composer)

    def push_src_data(self, src_name: str, data_packet):
        """
        push data to named appsrc
        :param src_name:
        :param data_packet
        :return:
        """
        pass

    def add_sink_callback(self, sink_name: str):
        """
        add callback for appsink data
        :param sink_name:
        :return:
        """
        pass
