# Composing a Pipeline

Default backend of AdaFlow is built on [GStreamer](https://gstreamer.freedesktop.org/), the de-facto media processing standard. GStreamer introduces the notion of pipeline to organize the data flow and behavior of processing units. And AdaFlow will follow this practice.

<img alt="Gstreamer Pipeline Example" src="./images/pipeline_example.png" width="800"/>

For better portability and pursuit of backend-agnostic, we create DSL (domain specific langauge) called `Universal Pipeline Lanaguage`. They are:

* written in JSON format, which is easy to learn and exchange.
* organized in a repository folder structure with some necessary conventions. 
* meant to be an isolation between user demands and different processing backends like `GStreamer`, `ffmpeg`, `Blender`, etc.


It often takes three steps to compose a pipeline:

1. Create local folder as pipeline repository.
2. Create `pipeline.json` for pipeline definitions.
3. Write extensions to complete your tasks if necessary.


To create a pipeline repository:

```shell
# to create a repository named `my_pipelines` and an empty pipeline definition for a pipeline named `foobar`.  
adaflow init my_pipelines foobar
```

Detailed instruction about our command-line interface is covered in [CLI](./cli.md). The `init` command is quite straight forward for repository initialization. 

Then modify `my_pipelines/foobar/pipeline.json` with actual pipeline definition. For example:

```
{
  "name": "foobar",
  "backend": "GStreamer",
  "dialect": [
    "{{F.source('src1')}}",
    "x264enc",
    "mp4mux",
    "{{F.sink('sink1')}}"
  ]
}
```

* `name` field is the unique pipeline name inside this repository.
* `backend` is the identifier for different processing backend. `GStreamer` is chosen here.
* `dialect` is the description of processing pipeline.
  * elements wrapped in ``{{`` and ```}}``` are runtime resolved elements. It will be dynamically compiled according to task requests. `F.source` and `F.sink` are placeholder function for source elements and sink elements.
  * `x264enc` element is used for X264 video encoding.
  * `mp4mux` element is used to re-mux frames into MP4 format. 

There are lots of built-in elements can be used in `dialect`. We will talk more about them in [Built-in elements](./built_in_elements.md). And the pipeline dialect syntax will be covered in [Concept](./concept.md).


Finally, to launch an ad-hoc execution for this pipeline:

```shell
adaflow launch my_pipelines foobar -d `{"sources": [{"name": "src1", "type": "file", "location": "file.mp4"}], "sinks": [{"name": "sink1", "type": "gst", "element": "filesink", "properties": {"location": "output.mp4"}}]}`
```

* we pass the actual task data using `-d` parameter, which accepts JSON string.
* In the task data:
  * a single file source is assigned with input from `file.mp4`
  * a single file sink is assigned, whose output is written to `output.mp4`.

To put it simple, it will decode a local file stored in `file.mp4` and then encode frames using X264 and then re-mux into a new MP4 file in `output.mp4`. In this example, no model related actions is taken, but it's a proper demonstration about IO abstraction and pipeline execution. To learn more about it, please refer to [Concept](./concept.md).