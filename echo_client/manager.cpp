/**
 * Copyright 2019 Nick Chadwick <nick@mux.com>. All rights reserved.
 * 
 * The glue between our PeerConnection and WebSocket interfaces.
 */

#include "manager.h"

#include <string>

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
#include <rtc_base/physical_socket_server.h>

class DummySetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
 public:
  static DummySetSessionDescriptionObserver* Create() {
    return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
  }
  virtual void OnSuccess() { std::cerr << "set session description observor called success" << std::endl; }
  virtual void OnFailure(webrtc::RTCError error) {
    std::cerr << "heck its a failure" << std::endl;
  }
};

Manager::Manager() {
    std::cout << "Manager creating" << std::endl;
}

Manager::~Manager() {
}

void Manager::InitializePeerConnectionFactory() {
    rtc::PhysicalSocketServer socket_server;
    rtc::AutoSocketServerThread aThread(&socket_server);

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
    }

    aThread.Run();
}

bool Manager::InitializePeerConnection() {
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    config.enable_dtls_srtp = true;
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = "stun:stun.l.google.com:19302";
    config.servers.push_back(server);

    std::cout << "Initializing peer connection" << std::endl;
    if (!this->peer_connection_factory) {
        std::cerr << "Shit's fucked" << std::endl;
    }

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
    std::cout << "OnSuccess" << std::endl;
    return;
}

void Manager::OnFailure(webrtc::RTCError error) {
    std::cout << "OnFailure" << std::endl;
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

    if (!this->peer_connection.get()) {
        std::cout << "Setting up the peer connection" << std::endl;
        if (!this->InitializePeerConnection()) {
            std::cerr << "ERROR: Failed to initalize peer connection" << std::endl;
            return;
        }
    }

    std::cout << "Parsing the message" << std::endl;

    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(message, jmessage)) {
        std::cerr << "Received unknown message. " << message << std::endl;
        return;
    }
    
    std::string msgType;-
    rtc::GetStringFromJsonObject(jmessage, "type", &msgType);
    if (msgType.empty()) {
        std::cerr << "ERROR: message type string is empty" << std::endl;
        return;
    }

    std::cerr << "Got msg of type: " << msgType << std::endl;

    if (msgType.compare("offer") == 0) {
        std::cerr << "Hey cool its a session description!" << std::endl;
        std::string sdpString;
        webrtc::SdpParseError error;
        if (!rtc::GetStringFromJsonObject(jmessage, "sdp", &sdpString)) {
            std::cerr << "ERROR: Message did not contain an SDP attribute" << std::endl;
            return;
        }

         std::unique_ptr<webrtc::SessionDescriptionInterface> offerDescription = webrtc::CreateSessionDescription(webrtc::SdpType::kOffer, sdpString, &error);
         if (!offerDescription) {
             std::cerr << "ERROR: Failed to parse incoming sdp offer: " << error.line << std::endl << error.description << std::endl;
             return;
         }

         std::cout << "Parsed SDP, generating answer" << std::endl;
         if (!this->peer_connection) {
             std::cerr << "um weird" << std::endl;
         }

         this->peer_connection->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), offerDescription.release());

         std::cerr << "Remote description set, generating an answer" << std::endl;
         this->peer_connection->CreateAnswer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());

    } else if (msgType.compare("candidate") == 0) {
        std::cerr << "Oh sweet an ice candidate" << std::endl;
    }
};

void Manager::OnWebsocketError() {
    std::cerr << "Oh noes, websocket error" << std::endl;
};
