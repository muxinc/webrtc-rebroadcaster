g++ -std=c++11 -DWEBRTC_POSIX main.cpp -I/opt/webrtc-checkout/src libwebrtc.a libwebrtc_common.a -lX11 -ldl -lpthread
