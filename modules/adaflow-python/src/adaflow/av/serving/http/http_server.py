# Copyright (c) Alibaba, Inc. and its affiliates.
import time
from logging.config import dictConfig
from flask import Flask, request, jsonify, url_for
import os, pathlib
from adaflow.av.pipeline.pipeline_factory import PipelineFactory
from adaflow.av.pipeline.dialects.gst_context import GstContext
from werkzeug.exceptions import HTTPException

# configure logging
dictConfig({
    'version': 1,
    'filters': {
        'request_id': {
            '()': "adaflow.av.serving.http.http_utils.RequestIdFilter",
        }
    },
    'formatters': {
        'default': {
            'format': '[%(asctime)s] [%(process)d] [%(levelname)s] [%(name)s.%(module)s.%(funcName)s:%(lineno)d] [%(request_id)s] %(message)s'
        }
    },
    'handlers': {'wsgi': {
        'class': 'logging.StreamHandler',
        'stream': 'ext://flask.logging.wsgi_errors_stream',
        'formatter': 'default',
        'filters': ['request_id']
    }},
    'root': {
        'level': 'INFO',
        'handlers': ['wsgi']
    }
})


def check_repository(repo_path):
    assert repo_path, "repo_path cannot be empty"
    assert pathlib.Path(repo_path).joinpath("pipelines").exists(), "pipelines folder should exist"


def validate_task_data(task_data: [str, any]):
    return task_data and "sinks" in task_data and "sources" in task_data


def create_http_server():
    # configure http server
    http_pipeline_server = Flask(__name__)
    # configure pipeline factory
    repo_path = os.environ.get("REPO_PATH", os.getcwd())
    check_repository(repo_path)
    http_pipeline_server.logger.info("initialize pipeline server with repo path %s" % repo_path)
    pipeline_factory = PipelineFactory.create(repository_path=pathlib.Path(repo_path))

    @http_pipeline_server.route("/health")
    def health():
        return "ok"

    @http_pipeline_server.errorhandler(404)
    def page_not_found(e):
        return {"error_message": "not found"}, 404

    @http_pipeline_server.errorhandler(Exception)
    def handle_error(e):
        code = 500
        if isinstance(e, HTTPException):
            code = e.code
        return {"error_message": str(e)}, code

    @http_pipeline_server.route("/pipelines/<pipeline_id>/launch", methods=["POST"])
    def launch_pipeline(pipeline_id):
        task_data = request.get_json(force=True)
        t1 = time.time_ns()
        http_pipeline_server.logger.info("start to lanch pipeline")
        if validate_task_data(task_data):
            builder = pipeline_factory.pipeline(pipeline_id).task(task_data).logger(http_pipeline_server.logger)
            with GstContext():
                with builder.build() as pipeline:
                    while not pipeline.is_done:
                        time.sleep(1)
        return {"elapsed": (time.time_ns() - t1) / 1000 / 1000}

    return http_pipeline_server
