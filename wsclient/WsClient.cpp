#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#define _SCL_SECURE_NO_WARNINGS
#endif // !ASIO_STANDALONE

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>
#include <string>
using namespace std;
#ifdef LINUX
#include "iconv.h"
#endif // LINUX

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::client<websocketpp::config::asio_client> client;

#ifdef WINDOWS
std::string UTF8_To_string(const std::string &str)
{
    int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t *pwBuf = new wchar_t[nwLen + 1];    //??????1??????????¦Â?? 
    memset(pwBuf, 0, nwLen * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);
    int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
    char *pBuf = new char[nLen + 1];
    memset(pBuf, 0, nLen + 1);
    WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

    std::string strRet = pBuf;

    delete[]pBuf;
    delete[]pwBuf;
    pBuf = NULL;
    pwBuf = NULL;

    return strRet;
}
#else
//UTF-8?gbk
string utf8_to_string(string strSrc)
{
    size_t srclen = strSrc.size() + 1;
    size_t dstlen = 2 * srclen;
    char *dst = new char[dstlen];
    char *pIn = (char *)strSrc.c_str();
    char *pOut = (char *)dst;
    iconv_t conv = iconv_open("utf-8", "gb2312");
    iconv(conv, &pIn, &srclen, &pOut, &dstlen);
    iconv_close(conv);
    return string(dst);
}
#endif

class CWsClient
{
public:
    CWsClient()
    {
        // Set logging to be pretty verbose (everything except message payloads)
        m_client.set_access_channels(websocketpp::log::alevel::all);
        m_client.clear_access_channels(websocketpp::log::alevel::frame_payload);
        m_client.set_error_channels(websocketpp::log::elevel::all);

        // Initialize ASIO
        m_client.init_asio();

        // Register our handlers
        m_client.set_message_handler(bind(&CWsClient::on_message, this, ::_1, ::_2));
        m_client.set_open_handler(bind(&CWsClient::on_open, this, ::_1));
        m_client.set_close_handler(bind(&CWsClient::on_close, this, ::_1));
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
        //m_client.send(hdl, "hello server1", websocketpp::frame::opcode::text);
    }

    void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg)
    {
#ifdef WINDOWS
        std::cout << "on_message: " << UTF8_To_string(msg->get_payload()) << std::endl;
#else
        std::cout << "on_message: " << utf8_to_string(msg->get_payload()) << std::endl;
#endif // WINDOWS

        
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
        CWsClient client;

        client.start(uri);
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}

