FROM ubuntu:14.04

ENV DEBIAN_FRONTEND noninteractive
RUN echo debconf shared/accepted-oracle-license-v1-1 select true | debconf-set-selections
RUN echo debconf shared/accepted-oracle-license-v1-1 seen true | debconf-set-selections

RUN apt-get update && apt-get install -y software-properties-common
RUN add-apt-repository ppa:webupd8team/java && add-apt-repository ppa:chris-lea/node.js && apt-get update

RUN apt-get install -y git subversion g++ python libnss3-dev libasound2-dev libpulse-dev libjpeg62-dev libxv-dev libgtk2.0-dev libexpat1-dev libxss-dev libudev-dev libdrm-dev libgconf2-dev libgcrypt11-dev libpci-dev libxtst-dev libgnome-keyring-dev libssl-dev nodejs oracle-java6-installer oracle-java6-set-default

ENV JAVA_HOME /usr/lib/jvm/java-6-oracle/
ENV PATH $PATH:/usr/lib/jvm/java-6-oracle/jre/bin/

ADD ./ /wrtc/
WORKDIR /wrtc/

RUN npm install && node_modules/.bin/node-pre-gyp install --fallback-to-build
RUN npm install
RUN npm test
