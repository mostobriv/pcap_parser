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
    libpq-dev

RUN git clone https://github.com/seladb/PcapPlusPlus.git

WORKDIR PcapPlusPlus
RUN ./configure-linux.sh --default
RUN make all
RUN make install

RUN git clone https://github.com/fmtlib/fmt /fmt
RUN mkdir -p /fmt/build
WORKDIR /fmt/build
RUN cmake ..
RUN make install

RUN mkdir /engine
WORKDIR /engine

ADD Makefile Makefile
ADD src/ src/

RUN make

ENTRYPOINT /bin/bash
