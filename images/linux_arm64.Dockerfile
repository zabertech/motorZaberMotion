FROM ubuntu:22.04

WORKDIR /opt

ENV TZ=America/Vancouver
ENV DEBIAN_FRONTEND=noninteractive

# c++
RUN apt-get update && apt-get install -y bash curl wget
RUN apt-get install -y build-essential git libreadline-dev libntirpc-dev re2c

# js
RUN curl -fsSL https://deb.nodesource.com/setup_16.x | bash - && apt-get install -y nodejs
RUN npm install -g npm@8.19.4
RUN apt-get install -y firefox

# add user
RUN apt-get install -y sudo
RUN useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1001 ubuntu
RUN echo "ubuntu ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
USER ubuntu

RUN mkdir -p $HOME/EPICS
RUN cd $HOME/EPICS \
    && wget https://epics-controls.org/download/base/base-7.0.8.1.tar.gz \
    && tar -xvf base-7.0.8.1.tar.gz \
    && cd base-7.0.8.1 \
    && make

RUN mkdir $HOME/EPICS/support
RUN cd $HOME/EPICS/support \
    && git clone https://github.com/epics-modules/sequencer \
    && git clone https://github.com/epics-modules/asyn \
    && git clone https://github.com/epics-modules/motor

ENV HOME=/home/ubuntu
ENV EPICS_BASE=$HOME/EPICS/base-7.0.8.1
RUN export EPICS_HOST_ARCH=$($EPICS_BASE/startup/EpicsHostArch) \
    && echo "export EPICS_HOST_ARCH=$EPICS_HOST_ARCH" >> /home/ubuntu/.bashrc
ENV PATH=$EPICS_BASE/bin/$EPICS_HOST_ARCH:$PATH
ENV CI_PROJECT_DIR=$HOME/EPICS/support/motor/modules

# RUN cd $HOME/EPICS/support/motor \
#     && git submodule add -b review-branch https://gitlab.izaber.com/colby.sparks/zaber-motor-epics.git modules/motorZaberMotion \
#     && cd $HOME/EPICS/support/motor/modules/motorZaberMotion \
#     && npm install \
#     && npx gulp update_epics_configs
