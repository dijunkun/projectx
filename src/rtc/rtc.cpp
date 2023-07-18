#include "rtc.h"

#include <iostream>

#include "ice_agent.h"
#include "log.h"
#include "nlohmann/json.hpp"
#include "peer_connection.h"
#include "ws_transport.h"

using nlohmann::json;

static PeerConnection *peer_connection;

int CreatePeerConnection(Params params) {
  PeerConnection::Params pc_params;
  pc_params.cfg_path = params.cfg_path;
  pc_params.on_receive_buffer = params.on_receive_buffer;
  pc_params.net_status_report = params.net_status_report;

  peer_connection = new PeerConnection();
  peer_connection->Init(pc_params);

  return 0;
}

int CreatePeerConnectionWithID(Params params, const char *id) {
  PeerConnection::Params pc_params;
  pc_params.cfg_path = params.cfg_path;
  pc_params.on_receive_buffer = params.on_receive_buffer;
  pc_params.net_status_report = params.net_status_report;

  peer_connection = new PeerConnection();
  peer_connection->Init(pc_params, id);

  return 0;
}

int SendData(const char *data, size_t size) {
  peer_connection->SendData(data, size);
  return 0;
}

int rtc() { return 0; }