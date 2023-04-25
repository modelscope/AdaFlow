import time

from flask import Flask, request
import os, pathlib
from ..pipeline.pipeline_factory import PipelineFactory
from ..pipeline.dialects.gst_context import GstContext

app = Flask(__name__)

repo_path = os.environ.get("REPO_PATH")
pipeline_factory = PipelineFactory.create(repository_path=pathlib.Path(repo_path))


def validate_task_data(task_data: [str, any]):
    return task_data and "sinks" in task_data and "sources" in task_data


@app.route("/health")
def health():
    return "ok"


@app.route("/pipelines/<pipeline_id>/launch", methods=["POST"])
def initialize_pipeline(pipeline_id):
    task_data = request.get_json(force=True)
    if validate_task_data(task_data):
        builder = pipeline_factory.pipeline(pipeline_id).task(task_data)
        with GstContext():
            with builder.build() as pipeline:
                while not pipeline.is_done:
                    time.sleep(1)
