# Copyright (c) Alibaba, Inc. and its affiliates.
from argparse import ArgumentParser
import logging
import time
from pathlib import Path
import json

from adaflow.av.pipeline.pipeline_factory import PipelineFactory
from adaflow.av.pipeline.dialects.gst_context import GstContext
from .base import CLICommand

logging.basicConfig(level=logging.DEBUG)


def subparser_func(args):
    """ Fuction which will be called for a specific sub parser.
    """
    return LaunchCMD(args)


class LaunchCMD(CLICommand):
    name = 'launch'

    def __init__(self, args):
        self.args = args

    @staticmethod
    def define_args(parsers: ArgumentParser):
        """ define args for launch command.
        """
        parser = parsers.add_parser(LaunchCMD.name)
        parser.add_argument('repo_path', type=str, help='Path of the repo to launch.')
        parser.add_argument('pipeline_name', type=str, help='Name of the pipeline to load.')
        parser.add_argument(
            '--task_path',
            type=str,
            default=None,
            help='JSON path of task.')
        parser.add_argument(
            '--task',
            type=str,
            default=None,
            help='dict of task command line.')
        parser.set_defaults(func=subparser_func)

    def pipeline_build(self):

        factory = PipelineFactory.create(Path(self.args.repo_path))
        builder = factory.pipeline(self.args.pipeline_name)

        if self.args.task_path is not None or self.args.task is not None:

            if self.args.task_path is not None:
                # open task-id
                with open(self.args.task_path) as j:
                    j.seek(0)
                    task_content = json.load(j)

            elif self.args.task is not None:

                task_content = json.loads(self.args.task)

            # add sources
            if "sources" in task_content:
                sources_sinks = task_content["sources"]
                for i in range(len(sources_sinks)):
                    builder.source(sources_sinks[i])

            # add sinks
            if "sinks" in task_content:
                task_sinks = task_content["sinks"]
                for i in range(len(task_sinks)):
                    builder.sink(task_sinks[i])

        else:
            logging.info("please make sure: complete pipeline description in pipeline JSON file")

        # run pipeline
        with GstContext():
            with builder.build() as pipeline:
                logging.info(pipeline.command)
                while not pipeline.is_done:
                    time.sleep(1)

    def execute(self):
        self.pipeline_build()
