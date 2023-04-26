import uuid
import logging
import flask
from werkzeug.datastructures import Headers

'''
Copied from
http://blog.mcpolemic.com/2016/01/18/adding-request-ids-to-flask.html
'''

# Generate a new request ID
def generate_request_id(headers: Headers):
    for key in ["X-Request-Id", "RequestId"]:
        if key in headers:
            return headers.get(key)
    return uuid.uuid4()

# Returns the current request ID or a new one if there is none
# In order of preference:
#   * If we've already created a request ID and stored it in the flask.g context local, use that
#   * If a client has passed in the X-Request-Id header, create a new ID with that prepended
#   * Otherwise, generate a request ID and store it in flask.g.request_id
def request_id():
    if getattr(flask.g, 'request_id', None):
        return flask.g.request_id

    headers = flask.request.headers
    new_uuid = generate_request_id(headers)
    flask.g.request_id = new_uuid

    return new_uuid

class RequestIdFilter(logging.Filter):


    # This is a logging filter that makes the request ID available for use in
    # the logging format. Note that we're checking if we're in a request
    # context, as we may want to log things before Flask is fully loaded.
    def filter(self, record):
        record.request_id = request_id() if flask.has_request_context() else ''
        return True