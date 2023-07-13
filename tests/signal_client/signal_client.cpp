#include "ws_client.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>

class WsReceiver:public WsCore
{
public:
    WsReceiver(){}
    ~WsReceiver(){}

    void OnReceiveMessage(const std::string &msg)
    {
        LOG_INFO("Receive msg: {}", msg);
    }
};

int main()
{
    bool done = false;
    std::string input;
    WsReceiver ws_client;

    LOG_INFO("connect ws://localhost:9002");
    ws_client.Connect("ws://localhost:9002");

    std::string status1 = ws_client.GetStatus();

    while("Open" != status1)
    {
        status1 = ws_client.GetStatus();
    }
    
    LOG_INFO("Connect successfully!");

    LOG_INFO("Send message [Hello]");
    ws_client.Send("Hello");

    LOG_INFO("Send ping");
    ws_client.Ping();

    LOG_INFO("Close conneciton");
    int close_code = websocketpp::close::status::normal;
    std::string reason = "User Close";
    ws_client.Close(close_code, reason);

    getchar();
    return 0;
}