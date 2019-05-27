
//------------------------------------------------------------------------------
//
// Example: WebSocket client, synchronous
//
//------------------------------------------------------------------------------

#include <api/scoped_refptr.h>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <api/peer_connection_interface.h>
#include <rtc_base/ssl_adapter.h>

#include "manager.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

void msg_writer(websocket::stream<tcp::socket>* ws) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        ws->write(net::buffer(std::string("{\"username\": \"the_beast\", \"msg\": \"ping\"}")));
    }
}

// Sends a WebSocket message and prints the response
int main(int argc, char** argv)
{
    if (!rtc::InitializeSSL()) {
        std::cout << "ah crap ssl init failed" << std::endl;
        return 1;
    }
    std::cout << "rtc ssl initialized" << std::endl;

    rtc::scoped_refptr<Manager> manager(new rtc::RefCountedObject<Manager>());

    try
    {
        auto const host = "localhost";
        auto const port = "5000";
        auto const text = "{\"username\":\"the_beast\", \"msg\":\"Hello! from native code!\"}";

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver{ioc};
        websocket::stream<tcp::socket> ws{ioc};

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        std::cout << "WebSocket connecting" << std::endl;
        // Make the connection on the IP address we get from a lookup
        net::connect(ws.next_layer(), results.begin(), results.end());

        // Set a decorator to change the User-Agent of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-coro");
            }));

        // Perform the websocket handshake
        ws.handshake(host, "/echo");

        // Send the message
        ws.write(net::buffer(std::string(text)));

        std::thread writer (msg_writer, &ws);

        while (1) {
            // This buffer will hold the incoming message
            beast::flat_buffer buffer;

            // Read a message into our buffer
            ws.read(buffer);

            // The make_printable() function helps print a ConstBufferSequence
            std::cout << beast::make_printable(buffer.data()) << std::endl;
        }
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
