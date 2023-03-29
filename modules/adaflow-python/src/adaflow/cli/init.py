# Copyright (c) Alibaba, Inc. and its affiliates.
from argparse import ArgumentParser
import logging
import os

from .base import CLICommand

logging.basicConfig(level=logging.DEBUG)


def subparser_func(args):
    """ Fuction which will be called for a specific sub parser.
    """
    return InitCMD(args)


class InitCMD(CLICommand):
    name = 'init'

    def __init__(self, args):
        self.args = args

    @staticmethod
    def define_args(parsers: ArgumentParser):
        """ define args for init command.
        """
        parser = parsers.add_parser(InitCMD.name)
        parser.add_argument('repo_name', type=str, help='Name of the repo to init.')
        parser.add_argument(
            '--pipeline',
            type=str,
            default='test',
            help='Name of the pipeline to init.')
        parser.set_defaults(func=subparser_func)

    def init_repo(self):

        if os.path.exists(self.args.repo_name):
            logging.info("%s exists" % self.args.repo_name)

        else:
            os.makedirs(self.args.repo_name)
            os.makedirs(os.path.join(self.args.repo_name, "pipelines"))
            os.makedirs(os.path.join(self.args.repo_name, "task"))
            os.makedirs(os.path.join(self.args.repo_name, "extension"))
            os.makedirs(os.path.join(self.args.repo_name, "resource"))
            logging.info("successfully create %s" % self.args.repo_name)

            if self.args.pipeline is not None:
                os.makedirs(os.path.join(self.args.repo_name, "pipelines", self.args.pipeline))
                os.makedirs(os.path.join(self.args.repo_name, "task", self.args.pipeline))
                logging.info("successfully create %s" % self.args.pipeline)

    def execute(self):
        self.init_repo()
