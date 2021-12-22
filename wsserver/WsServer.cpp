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
#ifdef LINUX
#include "iconv.h"
#endif // LINUX


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


#ifdef WINDOWS
// std::string 转换为 UTF-8 编码
std::string string_To_UTF8(const std::string &str)
{
    int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

    wchar_t *pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴 
    ZeroMemory(pwBuf, nwLen * 2 + 2);

    ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

    int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

    char *pBuf = new char[nLen + 1];
    ZeroMemory(pBuf, nLen + 1);

    ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

    std::string retStr(pBuf);

    delete[]pwBuf;
    delete[]pBuf;

    pwBuf = NULL;
    pBuf = NULL;

    return retStr;
}
#else
//gbk转UTF-8
string string_to_utf8(string strSrc)
{
    size_t srclen = strSrc.size() + 1;
    size_t dstlen = 2 * srclen;
    char *dst = new char[dstlen];
    char *pIn = (char *)strSrc.c_str();
    char *pOut = (char *)dst;
    iconv_t conv = iconv_open("gb2312", "utf-8");
    iconv(conv, &pIn, &srclen, &pOut, &dstlen);
    iconv_close(conv);
    return string(dst);
}
#endif

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
        m_webSocketServer.set_access_channels(websocketpp::log::alevel::all);
        m_webSocketServer.clear_access_channels(websocketpp::log::alevel::frame_payload);

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
        int64_t timestamp = time(0);
        string msg = "";
        string retMsg = "";
#ifdef WINDOWS
        msg += "timestamp" + Int_to_string(timestamp) + ",msg: 你好, 我是，";
        retMsg = string_To_UTF8(msg);
#else
        msg += "timestamp" + Int_to_string(timestamp) + ",msg: 你好, 我是，";
        retMsg = string_to_utf8(msg);
        cout << retMsg << endl;
        //msg = GbkToUtf8("你好，");
#endif // WINDOWS

        /*msg += R"({"timestamp":)" + Int_to_string(timestamp) +
            R"(, "id":"5555", "msg":"你好"})";*/

        return retMsg;
    }

    void send_messages()
    {
        int nSendTimes = 1;
        while (1)
        {
            string msg = generateMessage();
            msg = msg + ",send Times: " + Int_to_string(nSendTimes);

            for (auto &con : m_connections)
            {
                m_webSocketServer.send(
                    con, msg, websocketpp::frame::opcode::text);
            }
            nSendTimes++;
#ifdef WINDOWS
            Sleep(1000);
#else
            sleep(1);
#endif // WINDOWS
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
