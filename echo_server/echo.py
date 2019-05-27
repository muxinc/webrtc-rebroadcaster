#!/usr/bin/env python3
from flask import Flask, send_from_directory
from flask_sockets import Sockets

app = Flask(__name__)
sockets = Sockets(app)

@sockets.route('/echo')
def echo_socket(ws):
    while not ws.closed:
        message = ws.receive()
        ws.send(message)


@app.route('/')
def hello():
    return 'Hello world!'

@app.route('/app')
def send_app():
    return send_from_directory('static', 'index.html')

@app.route('/app/<path:path>')
def send_static(path):
    return send_from_directory('static', path)


if __name__ == "__main__":
    from gevent import pywsgi
    from geventwebsocket.handler import WebSocketHandler
    server = pywsgi.WSGIServer(('', 5000), app, handler_class=WebSocketHandler)
    server.serve_forever()
