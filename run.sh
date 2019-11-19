#!/bin/bash

docker-compose up --build && \
    docker run --rm -it \
    -v $PWD:/engine/app
    parser_engine_engine:latest
