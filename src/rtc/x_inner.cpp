#include "x_inner.h"

#include <iostream>
#include <nlohmann/json.hpp>

#include "ice_agent.h"
#include "log.h"
#include "ws_transmission.h"
#include "x.h"

using nlohmann::json;

static PeerConnection *peer_connection;

PeerPtr *CreatePeer(const Params *params) {
  PeerPtr *peer_ptr = new PeerPtr;
  peer_ptr->peer_connection = new PeerConnection();
  peer_ptr->pc_params.cfg_path = params->cfg_path;
  peer_ptr->pc_params.on_receive_buffer = params->on_receive_buffer;
  peer_ptr->pc_params.net_status_report = params->net_status_report;

  return peer_ptr;
}

// int CreateConnection(PeerPtr *peer_ptr) {
//   peer_ptr->peer_connection->Create(peer_ptr->pc_params);
//   return 0;
// }

int CreateConnection(PeerPtr *peer_ptr, const char *transmission_id,
                     const char *user_id) {
  peer_ptr->peer_connection->Create(peer_ptr->pc_params, transmission_id,
                                    user_id);
  return 0;
}

int JoinConnection(PeerPtr *peer_ptr, const char *transmission_id,
                   const char *user_id) {
  peer_ptr->peer_connection->Join(peer_ptr->pc_params, transmission_id,
                                  user_id);
  return 0;
}

int SendData(PeerPtr *peer_ptr, const char *data, size_t size) {
  peer_ptr->peer_connection->SendData(data, size);
  return 0;
}

int rtc() { return 0; }