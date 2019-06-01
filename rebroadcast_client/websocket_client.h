/**
 * All rights reserved n' shit.
 */

#ifndef WEBSOCKET_CLIENT_H_
#define WEBSOCKET_CLIENT_H_

#include <string>
#include <atomic>

#include <api/scoped_refptr.h>
#include <rtc_base/ref_count.h>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class WebsocketClientObserver {
public:
    virtual void OnDisconnected() = 0;
    virtual void OnMessage(const std::string& message) = 0;
    virtual void OnWebsocketError() = 0;
    virtual ~WebsocketClientObserver() {}
};

class WebsocketClient : public rtc::RefCountInterface {
public:
    WebsocketClient();
    void RegisterObserver(WebsocketClientObserver *observer);
    void Connect(const std::string &server, const std::string &port, const std::string &path);
    void Send(const std::string &msg);
    void Close();
    virtual ~WebsocketClient();

protected:
    void recv_loop();

    // Attributes
    WebsocketClientObserver *observer;
    net::io_context ioc;
    tcp::resolver resolver;
    websocket::stream<tcp::socket> ws;
    std::atomic<bool> should_close;
};

#endif // WEBSOCKET_CLIENT_H_