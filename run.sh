#!/bin/bash

docker-compose up --build && \
    docker run --rm -it \
    -v $PWD:/engine/app
    -p '127.0.0.1':8888:22
    parser_engine_engine:latest
