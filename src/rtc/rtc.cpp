#include "rtc.h"

#include <iostream>

#include "ice_agent.h"
#include "log.h"
#include "nlohmann/json.hpp"
#include "peer_connection.h"
#include "ws_transport.h"

using nlohmann::json;

static PeerConnection *peer_connection;

int CreatePeerConnection(const char *uri) {
  peer_connection = new PeerConnection();
  peer_connection->Init(uri);

  return 0;
}

int CreatePeerConnectionWithID(const char *uri, const char *id) {
  peer_connection = new PeerConnection();
  peer_connection->Init(uri, id);

  return 0;
}

int SendData(const char *data, size_t size) {
  peer_connection->SendData(data, size);
  return 0;
}

int rtc() { return 0; }