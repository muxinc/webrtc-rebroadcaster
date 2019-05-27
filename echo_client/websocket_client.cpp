/**
 * Copyright 2019 Nick Chadwick <nick@mux.com>. All rights reserved.
 */

#include <iostream>
#include <thread>

#include "websocket_client.h"

WebsocketClient::WebsocketClient() :
    ioc(),
    resolver(ioc),
    ws(ioc),
    should_close(false) {
    ;
}

WebsocketClient::~WebsocketClient() {
    ;
}

void WebsocketClient::RegisterObserver(WebsocketClientObserver *observer) {
    this->observer = observer;
}

void WebsocketClient::Connect(const std::string &server, const std::string &port, const std::string &path) {
    try {
        // Look up the domain name
        auto const results = this->resolver.resolve(server, port);

        // Make the connection on the IP address we get from a lookup
        net::connect(ws.next_layer(), results.begin(), results.end());

        // Perform the websocket handshake
        ws.handshake(server, path);

        // Start the recv_loop
        this->recv_loop();
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        this->observer->OnWebsocketError();
    }
}

void WebsocketClient::recv_loop() {
    while (!this->should_close) {
        // This buffer will hold the incoming message
        beast::flat_buffer buffer;

        // Read a message into our buffer
        ws.read(buffer);

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer.data()) << std::endl;

        if (buffer.size() > 0) {
            this->observer->OnMessage(beast::buffers_to_string(buffer.data()));
        } else {
            break;
        }
    }
    this->should_close = true;
    this->observer->OnDisconnected();
}

void WebsocketClient::Close() {
    this->ws.close(websocket::close_code::normal);
    this->should_close = true;
}

void WebsocketClient::Send(const std::string &msg) {
    try {
        this->ws.write(net::buffer(msg));
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error sending: " << e.what() << std::endl;
        this->should_close = true;
        this->observer->OnWebsocketError();
    }
}
