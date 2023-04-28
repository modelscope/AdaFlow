# Copyright (c) Alibaba, Inc. and its affiliates.
import logging
import os.path

from .base import CLICommand
from argparse import ArgumentParser
import multiprocessing
import gunicorn.app.base


class StandaloneApplication(gunicorn.app.base.BaseApplication):

    def __init__(self, app, options=None):
        self.options = options or {}
        self.application = app
        super().__init__()

    def load_config(self):
        config = {key: value for key, value in self.options.items()
                  if key in self.cfg.settings and value is not None}
        for key, value in config.items():
            self.cfg.set(key.lower(), value)

    def load(self):
        return self.application


def subparser_func(args):
    """ Fuction which will be called for a specific sub parser.
    """
    return ServeCMD(args)


class ServeCMD(CLICommand):
    """
    Command for starting pipeline server
    """
    name = "serve"

    def __init__(self, args):
        self.args = args

    @staticmethod
    def define_args(parsers: ArgumentParser):
        parser = parsers.add_parser(ServeCMD.name)
        parser.add_argument('--repo-path', type=str, help="path to pipeline repository", default=os.getcwd())
        parser.add_argument('--server-type', type=str,
                            help='server type for pipeline server. Currently only http is supported', default='http')
        parser.add_argument('--http-port', type=int, help='Port for http server', default=8080)
        parser.add_argument('--http-worker-num', type=int, default=1, help="Worker number for http server")
        parser.set_defaults(func=subparser_func)

    def execute(self):
        if os.path.exists(self.args.repo_path):
            from adaflow import create_http_server
            options = {
                'bind': '%s:%s' % ('127.0.0.1', self.args.http_port),
                'workers': self.args.http_worker_num,
            }
            logging.info(
                "Start to serve at port %s with %s worker(s)" % (self.args.http_port, self.args.http_worker_num))
            StandaloneApplication(create_http_server(), options).run()
        else:
            logging.error("repo path is not given")
