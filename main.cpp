#include <api/peerconnectioninterface.h>
#include <iostream>


int main(void) {
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pcfi = webrtc::CreatePeerConnectionFactory(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    std::cout << "Hello world!\n";
}
