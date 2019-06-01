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

Manager::Manager(rtc::scoped_refptr<WebsocketClient> ws) : ws(ws) {
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
    std::cout << "OnSignalingChange" << std::endl;
    return;
}

void Manager::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {
    return;
}

void Manager::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
    return;
}

void Manager::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) {
    std::cout << "OnDataChannel: " << channel->label() << std::endl;

    if (channel->label().compare("chat") == 0) {
        this->chatChannel = channel;
        this->chatChannel->RegisterObserver(this);
    }

    return;
}

void Manager::OnRenegotiationNeeded() {
    return;
}

void Manager::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
    std::cout << "OnIceConnectionChange" << std::endl;
    return;
}

void Manager::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
    std::cout << "OnIceGatheringChange" << std::endl;
    return;
}

void Manager::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
    std::string candidateStr;
    candidate->ToString(&candidateStr);
    std::cerr << "OnIceCandidate: " << candidateStr << std::endl;

    Json::Value candidateInfo;
    candidateInfo["candidate"] = candidateStr;
    candidateInfo["sdpMid"] = candidate->sdp_mid();
    candidateInfo["sdpMLineIndex"] = candidate->sdp_mline_index();

    Json::Value candidateMsg;
    candidateMsg["type"] = "ice-candidate";
    candidateMsg["candidate"] = candidateInfo;

    Json::StyledWriter writer;
    this->ws->Send(writer.write(candidateMsg));

    std::cerr << "Sent ice-candidate" << std::endl;

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
    this->peer_connection->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);

    std::string sdp;
    desc->ToString(&sdp);

    std::cout << "This is the sdp: " << sdp << std::endl;

    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage["type"] = webrtc::SdpTypeToString(desc->GetType());
    jmessage["sdp"] = sdp;

    std::string msg = writer.write(jmessage);
    std::cout << "I'm sending this as a response: " << msg << std::endl;

    this->ws->Send(msg);

    return;
}

void Manager::OnFailure(webrtc::RTCError error) {
    std::cout << "CreateSessionDescriptionObserver OnFailure" << std::endl;
    return;
}


//
// DataChannelObserver implementation.
//
void Manager::OnMessage(const webrtc::DataBuffer& buffer) {
    std::string msg(buffer.data.data<char>(), buffer.data.size());
    std::cout << "Received message: " << msg << std::endl;
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

    } else if (msgType.compare("ice-candidate") == 0) {
        std::cerr << "Oh sweet an ice candidate" << std::endl;

        Json::Value candidateMsg;
        if (!rtc::GetValueFromJsonObject(jmessage, "candidate", &candidateMsg)) {
            std::cerr << "ERROR: failed to get 'candidate' attribute of ice-candidate message" << std::endl;
            return;
        }

        std::string sdpMid;
        int sdpMlineindex = 0;
        std::string candidateSdp;
        if (!rtc::GetStringFromJsonObject(candidateMsg, "sdpMid", &sdpMid)) {
            std::cerr << "ERROR: Failed to get 'sdpMid' from ice-candidate message" << std::endl;
            return;
        }
        if (!rtc::GetIntFromJsonObject(candidateMsg, "sdpMLineIndex", &sdpMlineindex)) {
            std::cerr << "ERROR: Failed to get 'sdpMLineIndex' from ice-candidate message" << std::endl;
            return;
        }
        if (!rtc::GetStringFromJsonObject(candidateMsg, "candidate", &candidateSdp)) {
            std::cerr << "ERROR: Failed to get 'candidate' from ice-candidate message" << std::endl;
            return;
        }

        webrtc::SdpParseError error;
        std::unique_ptr<webrtc::IceCandidateInterface> candidate(webrtc::CreateIceCandidate(sdpMid, sdpMlineindex, candidateSdp, &error));
        if (!candidate.get()) {
            std::cerr << "Can't parse received candidate message. " << "SdpParseError was: " << error.description << std::endl;
            return;
        }

        if (!this->peer_connection->AddIceCandidate(candidate.get())) {
            std::cerr << "Failed to apply the received candidate" << std::endl;
            return;
        }
    }
};

void Manager::OnWebsocketError() {
    std::cerr << "Oh noes, websocket error" << std::endl;
};
