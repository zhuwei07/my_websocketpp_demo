#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#define _SCL_SECURE_NO_WARNINGS
#endif // !ASIO_STANDALONE

#include <chrono>
#include <cstring>
#include <ctime>
#include <fstream>
#include <future>
#include <iostream>
#include <mutex>
#include <set>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <time.h>
#include <vector>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>

using namespace std;
// using ws = websocketpp;

using server = websocketpp::server<websocketpp::config::asio>;
using Connections = std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>>;
std::mutex conMutex;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;


string Int_to_string(int n)
{
    ostringstream stream;
    stream << n;
    return stream.str();
}

class CWsServer
{
public:
    CWsServer()
    {
        // Initialize Asio Transport
        m_webSocketServer.init_asio();

        // Register handler callbacks
        m_webSocketServer.set_open_handler(bind(&CWsServer::on_open, this, ::_1));
        m_webSocketServer.set_close_handler(bind(&CWsServer::on_close, this, ::_1));
        m_webSocketServer.set_message_handler(bind(&CWsServer::on_message, this, ::_1, ::_2));
    }

    void run(uint16_t port)
    {
        // listen on specified port
        m_webSocketServer.listen(port);

        // Start the server accept loop
        m_webSocketServer.start_accept();

        // Start the ASIO io_service run loop
        try
        {
            m_webSocketServer.run();
        }
        catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }

    void on_open(connection_hdl hdl)
    {
        cout << "on_open" << endl;
        lock_guard<mutex> lock(conMutex);
        m_connections.insert(hdl);
    }

    void on_close(connection_hdl hdl)
    {
        cout << "on_close" << endl;
        lock_guard<mutex> lock(conMutex);
        m_connections.erase(hdl);
    }

    void on_message(connection_hdl hdl, server::message_ptr msg)
    {
        cout << "on_message" << endl;
        std::cout << "server received: " + msg->get_payload() << std::endl;
    }

    string generateMessage()
    {
        static vector<unsigned char> buffer;

        int64_t timestamp = time(0);
        string msg = "";
        msg += R"({"timestamp":)" + Int_to_string(timestamp) +
            R"(, "id":"5555", "payload":"hello world"})";

        return msg;
    }

    void send_messages()
    {
        int nSendTimes = 1;
        while (1)
        {
            string msg = generateMessage();
            msg = msg + "send Times: " + Int_to_string(nSendTimes);

            lock_guard<mutex> lock(conMutex);
            for (auto &con : m_connections)
            {
                m_webSocketServer.send(
                    con, msg, websocketpp::frame::opcode::text);
            }
            nSendTimes++;
            Sleep(1000);
        }
    }

private:
    server m_webSocketServer;
    Connections m_connections;
};

int main(int argc, char **argv)
{
    try
    {
        CWsServer server_instance;

        thread t(bind(&CWsServer::send_messages, &server_instance));

        // Run the asio loop with the main thread
        server_instance.run(9002);
        //启动发送消息的线程
        t.join();
    }
    catch (websocketpp::exception const &e)
    {
        std::cout << e.what() << std::endl;
    }
}
