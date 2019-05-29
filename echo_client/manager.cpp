/**
 * Copyright 2019 Nick Chadwick <nick@mux.com>. All rights reserved.
 * 
 * The glue between our PeerConnection and WebSocket interfaces.
 */

#include "manager.h"

#include <api/create_peerconnection_factory.h>
#include <api/audio_codecs/audio_decoder_factory.h>
#include <api/audio_codecs/audio_encoder_factory.h>
#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
#include <api/video_codecs/builtin_video_decoder_factory.h>
#include <api/video_codecs/builtin_video_encoder_factory.h>
#include <api/video_codecs/video_decoder_factory.h>
#include <api/video_codecs/video_encoder_factory.h>
#include <rtc_base/strings/json.h>

Manager::Manager() {
    std::cout << "Manager creating" << std::endl;
}

Manager::~Manager() {
}

bool Manager::InitializePeerConnection() {
    this->peer_connection_factory = webrtc::CreatePeerConnectionFactory(
      nullptr /* network_thread */, nullptr /* worker_thread */,
      nullptr /* signaling_thread */, nullptr /* default_adm */,
      webrtc::CreateBuiltinAudioEncoderFactory(),
      webrtc::CreateBuiltinAudioDecoderFactory(),
      webrtc::CreateBuiltinVideoEncoderFactory(),
      webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */,
      nullptr /* audio_processing */);
    
    if (!this->peer_connection_factory) {
        std::cerr << "Failed to initialize peer connection factory" << std::endl;
        return false;
    }

    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    config.enable_dtls_srtp = true;
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = "stun:stun.l.google.com:19302";
    config.servers.push_back(server);

    this->peer_connection = this->peer_connection_factory->CreatePeerConnection(config, nullptr, nullptr, this);

    return this->peer_connection != nullptr;
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

    if (!this->peer_connection_factory.get()) {
        this->InitializePeerConnection();
    }

    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(message, jmessage)) {
        std::cerr << "Received unknown message. " << message << std::endl;
        return;
    }
    
    std::string type_str;
    rtc::GetStringFromJsonObject(jmessage, "type", &type_str);
    if (!type_str.empty()) {
        std::cerr << "Got msg of type: " << type_str << std::endl;
    } else {
        std::cerr << "Ah shit the type string is empty" << std::endl;
    }
};

void Manager::OnWebsocketError() {
    std::cerr << "Oh noes, websocket error" << std::endl;
};
