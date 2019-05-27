/**
 * Copyright 2019 Nick Chadwick <nick@mux.com>. All rights reserved.
 * 
 * The glue between our PeerConnection and WebSocket interfaces.
 */

#include "manager.h"



Manager::Manager() {
    std::cout << "Manager creating" << std::endl;
}

Manager::~Manager() {
}


//
// PeerConnectionObserver implementation.
//

void Manager::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {
    return;
}

void Manager::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {
    return;
}

void Manager::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
    return;
}

void Manager::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) {
    return;
}

void Manager::OnRenegotiationNeeded() {
    return;
}

void Manager::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
    return;
}

void Manager::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
    return;
}

void Manager::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
    return;
}

void Manager::OnIceConnectionReceivingChange(bool receiving) {
    return;
}

//
// CreateSessionDescriptionObserver implementation.
//
void Manager::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
    return;
}

void Manager::OnFailure(webrtc::RTCError error) {
    return;
}

//
// WebsocketClient implementation.
//

void Manager::OnDisconnected() {
    std::cerr << "Websocket disconnected" << std::endl;
};

void Manager::OnMessage(const std::string& message) {
    std::cout << "Got a message: " << message << std::endl;
};

void Manager::OnWebsocketError() {
    std::cerr << "Oh noes, websocket error" << std::endl;
};
