FROM ubuntu:22.04

WORKDIR /opt

ENV TZ=America/Vancouver
ENV DEBIAN_FRONTEND=noninteractive

# c++
RUN apt-get update && apt-get install -y bash curl wget unzip
RUN apt-get install -y build-essential cmake git libreadline-dev re2c

# js
RUN curl -fsSL https://deb.nodesource.com/setup_16.x | bash - && apt-get install -y nodejs
RUN npm install -g npm@8.19.4
RUN apt-get install -y firefox

# add user and set home directory
RUN apt-get install -y sudo
RUN useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1001 ubuntu
RUN echo "ubuntu ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
USER ubuntu
ENV HOME=/home/ubuntu

# py
RUN curl -LsSf https://astral.sh/uv/install.sh | sh

# get and build epics base
RUN mkdir -p $HOME/EPICS
RUN cd $HOME/EPICS \
    && wget https://epics-controls.org/download/base/base-7.0.8.1.tar.gz \
    && tar -xvf base-7.0.8.1.tar.gz \
    && cd base-7.0.8.1 \
    && make

# add motor support module and its dependencies
# specific versions are those recommended in motor's configure/RELEASE file.
# if you want to update these versions, please refer to that file:
# https://github.com/epics-modules/motor/blob/R7-4/configure/RELEASE
RUN mkdir $HOME/EPICS/support
RUN cd $HOME/EPICS/support \
    && git clone --branch R7-4    https://github.com/epics-modules/motor \
    && git clone --branch R4-35   https://github.com/epics-modules/asyn \
    && git clone --branch R2-2-6  https://github.com/epics-modules/sequencer \
    && git clone --branch R1-7-1  https://github.com/epics-modules/busy

# set env vars for EPICS and UV
ENV EPICS_BASE=$HOME/EPICS/base-7.0.8.1
RUN export EPICS_HOST_ARCH=$($EPICS_BASE/startup/EpicsHostArch) \
    && echo "export EPICS_HOST_ARCH=$EPICS_HOST_ARCH" >> /home/ubuntu/.bashrc
ENV PATH=$EPICS_BASE/bin/$EPICS_HOST_ARCH:$PATH
ENV PATH="$HOME/.local/bin:$PATH"
