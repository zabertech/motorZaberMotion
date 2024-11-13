FROM ubuntu:22.04

WORKDIR /opt

ENV TZ=America/Vancouver
ENV DEBIAN_FRONTEND=noninteractive

# c++
RUN apt-get update && apt-get install -y bash curl wget
RUN apt-get install -y build-essential git libreadline-dev

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
RUN cd $HOME/EPICS
RUN wget https://epics-controls.org/download/base/base-7.0.8.1.tar.gz
RUN tar -xvf base-7.0.8.1.tar.gz

RUN mkdir $HOME/EPICS/support
RUN cd $HOME/EPICS/support
RUN git clone https://github.com/epics-modules/sequencer
RUN git clone https://github.com/epics-modules/asyn
RUN git clone https://github.com/epics-modules/motor

RUN export EPICS_BASE=$HOME/EPICS/base-7.0.8.1
RUN export EPICS_HOST_ARCH=$($EPICS_BASE/startup/EpicsHostArch)
RUN export PATH=$EPICS_BASE/bin/$EPICS_HOST_ARCH:$PATH
RUN export CI_PROJECT_DIR=$HOME/EPICS/support/motor/modules

# TODO: this should be added to gitlab ci script after testing
RUN cd $HOME/EPICS/support/motor
RUN git submodule add -b review-branch git@gitlab.izaber.com:colby.sparks/zaber-motor-epics.git modules/motorZaberMotion
RUN cd $HOME/EPICS/support/motor/modules/motorZaberMotion
RUN npm install
RUN npx gulp update_epics_configs
