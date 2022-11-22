FROM centos:7
RUN yum install -y openssl openssl-devel \
&& yum groupinstall -y Development Tools \
&& curl -O https://nodejs.org/dist/v14.15.0/node-v14.15.0-linux-x64.tar.xz \
&& tar -xf node-v14.15.0-linux-x64.tar.xz \
&& cd node-v14.15.0-linux-x64/bin && ls \
&& ln -s /__w/node-webrtc/node-webrtc/node-v14.15.0-linux-x64/bin/node /usr/bin/node \
&& ln -s /__w/node-webrtc/node-webrtc/node-v14.15.0-linux-x64/bin/npm /usr/bin/npm \
&& cd .. \
&& curl -O https://cmake.org/files/v3.12/cmake-3.12.4.tar.gz \
&& tar zxf cmake-3.12.4.tar.gz \
&& cd cmake-3.12.4 \
&& ./bootstrap \
&& gmake -j 4 \
&& gmake install \
&& cd .. \
&& yum install -y python36 \
&& curl -O http://ftp.gnu.org/gnu/glibc/glibc-2.18.tar.gz \
&& tar zxf glibc-2.18.tar.gz \
&& cd glibc-2.18 \
&& mkdir build \
&& cd build \
&& ../configure --prefix=/usr --disable-profile --enable-add-ons --with-headers=/usr/include --with-binutils=/usr/bin \
&& make -j 4 \
&& make install 
