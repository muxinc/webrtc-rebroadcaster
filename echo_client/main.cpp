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

#include "websocket_client.h"
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

    rtc::scoped_refptr<WebsocketClient> ws(new rtc::RefCountedObject<WebsocketClient>());
    rtc::scoped_refptr<Manager> manager(new rtc::RefCountedObject<Manager>(ws));


    std::thread pcThread(&Manager::InitializePeerConnectionFactory, manager);

    ws->RegisterObserver(manager);
    ws->Connect("localhost", "5000", "/echo");

    std::cerr << "Done, returning" << std::endl;
    pcThread.join();

    return 0;
}
