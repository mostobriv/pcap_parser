FROM ubuntu:19.04

RUN apt-get update && \
    apt-get install -qy \
    libpcap-dev \
    git \
    make \
    cmake \
    g++ \
    gcc \
    libboost-stacktrace-dev \
    libboost-system-dev \
    libboost-filesystem-dev \
    libpq-dev \
    libpqxx-dev

RUN mkdir /engine
WORKDIR /engine
COPY .git/ .git/
COPY .gitmodules .gitmodules

COPY Makefile Makefile
RUN mkdir -p lib/PcapPlusPlus/
RUN mkdir -p lib/fmt/

RUN make lib/fmt/build/
RUN make lib/PcapPlusPlus/mk/platform.mk

COPY src/ src/
RUN make

ENTRYPOINT /bin/bash
