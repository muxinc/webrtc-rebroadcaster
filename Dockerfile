FROM ubuntu:xenial

RUN apt-get update && apt-get install -y git curl wget build-essential python lsb-release sudo
RUN git config --global user.name "John Doe" && \
    git config --global user.email "jdoe@email.com" && \
    git config --global core.autocrlf false && \
    git config --global core.filemode false && \
    git config --global color.ui true

RUN cd /opt && git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
RUN echo "export PATH=$PATH:/opt/depot_tools" >> ~/.bashrc
ENV PATH="/opt/depot_tools:${PATH}"

ENV DEBIAN_FRONTEND=noninteractive
ENV DEBCONF_NONINTERACTIVE_SEEN=true
RUN apt-get install -y tzdata ttf-mscorefonts-installer

RUN mkdir /opt/webrtc-checkout && \
    cd /opt/webrtc-checkout && \
    fetch --nohooks webrtc && \
    cd /opt/webrtc-checkout/src && \
    git checkout master && \
    cd /opt/webrtc-checkout && \
    gclient sync

WORKDIR /opt/webrtc-checkout/src/
RUN cat ./build/install-build-deps.sh
RUN cd /opt/webrtc-checkout/src && DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true ./build/install-build-deps.sh --no-prompt

RUN gn gen out/Debug66 --args="rtc_include_tests=false rtc_use_h264=false use_rtti=true is_component_build=false enable_iterator_debugging=false enable_nacl=false target_os=\"linux\" target_cpu=\"x64\" is_debug=true use_custom_libcxx=false use_custom_libcxx_for_host=false"
RUN ninja -C out/Debug66

