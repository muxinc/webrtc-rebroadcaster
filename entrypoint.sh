#!/bin/bash

cd /opt/rebroadcast_server && python3 serve.py &

sleep 1

pulseaudio -D

sleep 1

cd /opt/rebroadcast_client/build && ./main
