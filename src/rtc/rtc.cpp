#include "rtc.h"

#include <iostream>
#include <nlohmann/json.hpp>

#include "ice_agent.h"
#include "log.h"
#include "peer_connection.h"
#include "ws_transport.h"

using nlohmann::json;

static const std::vector<std::string> siganl_status = {"Connecting",
                                                       "Connected", "Closed"};

class WsSender : public WsCore {
 public:
  WsSender() {}
  ~WsSender() {}

  void OnReceiveMessage(const std::string &msg) {
    LOG_INFO("Receive msg: {}", msg);
  }
};

static WsSender *ws_client;
static PeerConnection *peer_connection;

int CreatePeerConnection(const char *uri) {
  peer_connection = new PeerConnection();
  peer_connection->Init(uri);

  // do {
  // } while (SignalStatus::Connected != peer_connection->GetSignalStatus());

  // LOG_INFO("Signal status: {}",
  //          siganl_status[peer_connection->GetSignalStatus()]);

  // peer_connection->CreateTransport();
  // peer_connection->CreateOffer();

  return 0;
}

int CreatePeerConnectionWithID(const char *uri, const char *id) {
  peer_connection = new PeerConnection();
  peer_connection->Init(uri, id);

  // do {
  // } while (SignalStatus::Connected != peer_connection->GetSignalStatus());

  // LOG_INFO("Signal status: {}",
  //          siganl_status[peer_connection->GetSignalStatus()]);

  // peer_connection->CreateTransport(id);

  return 0;
}

int rtc() {
  ws_client = new WsSender();
  return 0;
}

int CreateWsClient(const char *uri) {
  ws_client->Connect(uri);

  return 0;
}

int WsSendMsg(const char *message) {
  ws_client->Send(message);

  return 0;
}

// ws_status GetWsStatus()
// {
//     std::string ws_status = ws_client->GetStatus();

//     if ("Connecting" == ws_status)
//     {
//         return ws_status::WS_CONNECTING;
//     }
//     else if ("Open" == ws_status)
//     {
//         return ws_status::WS_OPEN;
//     }
//     else if ("Failed" == ws_status)
//     {
//         return ws_status::WS_FAILED;
//     }
//     else if ("Closed" == ws_status)
//     {
//         return ws_status::WS_CLOSED;
//     }
//     else
//     {
//         return ws_status::WS_UNKNOWN;
//     }
// }