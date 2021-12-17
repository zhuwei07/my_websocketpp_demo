#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#define _SCL_SECURE_NO_WARNINGS
#endif // !ASIO_STANDALONE

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::client<websocketpp::config::asio_client> client;

class send_msg_to_broad
{
public:
    send_msg_to_broad()
    {
        // Set logging to be pretty verbose (everything except message payloads)
        m_client.set_access_channels(websocketpp::log::alevel::all);
        m_client.clear_access_channels(websocketpp::log::alevel::frame_payload);
        m_client.set_error_channels(websocketpp::log::elevel::all);

        // Initialize ASIO
        m_client.init_asio();

        // Register our handlers
        m_client.set_message_handler(bind(&send_msg_to_broad::on_message, this, ::_1, ::_2));
        m_client.set_open_handler(bind(&send_msg_to_broad::on_open, this, ::_1));
        m_client.set_close_handler(bind(&send_msg_to_broad::on_close, this, ::_1));
    }

    void start(std::string uri) {
        websocketpp::lib::error_code ec;
        client::connection_ptr con = m_client.get_connection(uri, ec);
        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return;
        }

        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        m_client.connect(con);

        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        m_client.run();
    }

    void on_open(websocketpp::connection_hdl hdl)
    {
        std::cout << "on_open" << std::endl;
        m_client.send(hdl, "hello server1", websocketpp::frame::opcode::text);
    }

    void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg)
    {
        std::cout << "on_message: " << msg->get_payload() << std::endl;
    }

    void on_close(websocketpp::connection_hdl hdl)
    {
        std::cout << "on_close" << std::endl;
    }

private:
    client m_client;
};

int main(int argc, char* argv[]) {

    std::string uri = "ws://localhost:9002";

	try {
        send_msg_to_broad client;

        client.start(uri);
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}

