# Copyright (c) Alibaba, Inc. and its affiliates.
import argparse

from .launch import LaunchCMD
from .init import InitCMD


def run_cmd():
    parser = argparse.ArgumentParser(
        'AdaFlow Command Line tool', usage='adaflow <command> [<args>]')
    subparsers = parser.add_subparsers(help='adaflow commands helpers')

    LaunchCMD.define_args(subparsers)
    InitCMD.define_args(subparsers)

    args = parser.parse_args()

    if not hasattr(args, 'func'):
        parser.print_help()
        exit(1)

    cmd = args.func(args)
    cmd.execute()


if __name__ == '__main__':
    run_cmd()
