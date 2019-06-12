/**
 * Copyright 2019 Nick Chadwick <nick@mux.com>. All rights reserved.
 * 
 * The glue between our PeerConnection and WebSocket interfaces.
 */

#ifndef MANAGER_H_
#define MANAGER_H_

#include <iostream>
#include <api/media_stream_interface.h>
#include <api/peer_connection_interface.h>

#include "websocket_client.h"
#include "transcoder.h"

class Manager : public webrtc::PeerConnectionObserver,
                public webrtc::CreateSessionDescriptionObserver,
                public webrtc::DataChannelObserver,
                public WebsocketClientObserver {
public:
    Manager(rtc::scoped_refptr<WebsocketClient> ws);
    virtual ~Manager();

    void InitializePeerConnectionFactory();
    bool InitializePeerConnection();

    //
    // PeerConnectionObserver implementation.
    //

    void OnSignalingChange(
        webrtc::PeerConnectionInterface::SignalingState new_state) override;

    void OnRemoveTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;

    void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
    void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

    void OnDataChannel(
        rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
    void OnRenegotiationNeeded() override;
    void OnIceConnectionChange(
        webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
    void OnIceGatheringChange(
        webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
    void OnIceConnectionReceivingChange(bool receiving) override;

    //
    // CreateSessionDescriptionObserver implementation.
    //
    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    void OnFailure(webrtc::RTCError error) override;

    //
    // DataChannelObserver implementation.
    //
    void OnStateChange() override {};
    void OnMessage(const webrtc::DataBuffer& buffer) override;

    //
    // WebsocketClient implementation.
    //
    void OnDisconnected() override;
    void OnMessage(const std::string& message) override;
    void OnWebsocketError() override;

private:
    std::string rtmpURL;
    rtc::scoped_refptr<WebsocketClient> ws;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection;

    rtc::scoped_refptr<webrtc::DataChannelInterface> chatChannel;

    std::shared_ptr<Transcoder> transcoder;
};

#endif // MANAGER_H_