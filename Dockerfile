FROM ubuntu:xenial

RUN apt-get update && apt-get install -y git curl wget build-essential python lsb-release sudo
RUN git config --global user.name "John Doe" && \
    git config --global user.email "jdoe@email.com" && \
    git config --global core.autocrlf false && \
    git config --global core.filemode false && \
    git config --global color.ui true

#
# Build WebRTC
#
# TODO: Re-enable and use built artefacts
#RUN cd /opt && git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
#RUN echo "export PATH=$PATH:/opt/depot_tools" >> ~/.bashrc
#ENV PATH="/opt/depot_tools:${PATH}"

#ENV DEBIAN_FRONTEND=noninteractive
#ENV DEBCONF_NONINTERACTIVE_SEEN=true
#RUN apt-get install -y tzdata ttf-mscorefonts-installer

#RUN mkdir /opt/webrtc-checkout && \
#    cd /opt/webrtc-checkout && \
#    fetch --nohooks webrtc && \
#    cd /opt/webrtc-checkout/src && \
#    git checkout branch-heads/m75 && \
#    gclient sync && \
#    cd /opt/webrtc-checkout/src && \
#    DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true ./build/install-build-deps.sh --no-prompt && \
#    gn gen out/Release72 --args="rtc_include_tests=false rtc_use_h264=false use_rtti=true is_component_build=false enable_iterator_debugging=false target_os=\"linux\" target_cpu=\"x64\" is_d    ebug=false use_custom_libcxx=false use_custom_libcxx_for_host=false" && \
#    ninja -C out/Release72 && \
#    cd /opt/webrtc-checkout/src/out/Release72/obj/third_party/jsoncpp/jsoncpp && ar rcs ../../libjsoncpp.a json_reader.o json_writer.o json_value.o && \
#    mv /opt/webrtc-checkout/src/out/Release72/obj /opt/webrtc-build && \
#    rm -r /opt/webrtc-checkout/

#
# Install FFmpeg
#

RUN apt-get update && apt-get install -y software-properties-common wget
RUN add-apt-repository ppa:jonathonf/ffmpeg-4 && apt-get update && apt-get install -y ffmpeg libavcodec-dev libavformat-dev libavutil-dev

#
# Install deps
#
RUN apt-get update && apt-get install -y cmake g++ python-dev autotools-dev libicu-dev libbz2-dev libssl-dev


#
# Install Boost
#

RUN mkdir /opt/boost
WORKDIR /opt/boost

RUN cd /opt && \
    wget "https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz" && \
    tar xvzf boost_1_70_0.tar.gz && \
    rm boost_1_70_0.tar.gz && \
    cd boost_1_70_0 && \
    ./bootstrap.sh && \
    ./b2 -j 4; \
    ./b2 install && \
    cd /opt && \
    rm -rf /opt/boost_1_70_0

#
# Build rebroadcast client
#

RUN cd /opt && git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
RUN echo "export PATH=$PATH:/opt/depot_tools" >> ~/.bashrc
ENV PATH="/opt/depot_tools:${PATH}"

RUN mkdir /opt/webrtc-checkout && \
    cd /opt/webrtc-checkout && \
    fetch --nohooks webrtc && \
    cd /opt/webrtc-checkout/src && \
    git checkout branch-heads/m75 && \
    gclient sync

RUN apt-get update && apt-get install -y libx11-dev

ADD rebroadcast_client/ /opt/rebroadcast_client
WORKDIR /opt/rebroadcast_client
RUN cd /opt/rebroadcast_client && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make

ADD rebroadcast_server /opt/rebroadcast_server
RUN apt-get update && apt-get install -y python3-pip && \
    pip3 install Flask Flask-Sockets

#
# Install PulseAudio
# TODO: Build WebRTC with dummy audio (so that PulseAudio isn't a requirement)
#

RUN apt-get update && apt-get install -y alsa-utils pulseaudio && \
    echo "pcm.default pulse\nctl.default pulse" > .asoundrc

#
# Install entrypoint
#

ADD entrypoint.sh /opt/entrypoint.sh
ENTRYPOINT ["/bin/bash", "/opt/entrypoint.sh"]

