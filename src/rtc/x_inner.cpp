#include "x_inner.h"

#include <iostream>
#include <nlohmann/json.hpp>

#include "ice_agent.h"
#include "log.h"
#include "ws_transmission.h"
#include "x.h"

using nlohmann::json;

PeerPtr *CreatePeer(const Params *params) {
  PeerPtr *peer_ptr = new PeerPtr;
  peer_ptr->peer_connection = new PeerConnection();
  peer_ptr->pc_params.cfg_path = params->cfg_path;
  peer_ptr->pc_params.on_receive_video_buffer = params->on_receive_video_buffer;
  peer_ptr->pc_params.on_receive_audio_buffer = params->on_receive_audio_buffer;
  peer_ptr->pc_params.on_receive_data_buffer = params->on_receive_data_buffer;
  peer_ptr->pc_params.on_signal_status = params->on_signal_status;
  peer_ptr->pc_params.on_connection_status = params->on_connection_status;
  peer_ptr->pc_params.net_status_report = params->net_status_report;

  return peer_ptr;
}

int Init(PeerPtr *peer_ptr, const char *user_id) {
  if (!peer_ptr) {
    LOG_ERROR("peer_ptr not created");
    return -1;
  }

  peer_ptr->peer_connection->Init(peer_ptr->pc_params, user_id);
  return 0;
}

int CreateConnection(PeerPtr *peer_ptr, const char *transmission_id,
                     const char *password) {
  if (!peer_ptr) {
    LOG_ERROR("peer_ptr not created");
    return -1;
  }

  LOG_INFO("CreateConnection [{}] with password [{}]", transmission_id,
           password);

  return peer_ptr->peer_connection->Create(transmission_id, password);
}

int JoinConnection(PeerPtr *peer_ptr, const char *transmission_id,
                   const char *password) {
  if (!peer_ptr) {
    LOG_ERROR("peer_ptr not created");
    return -1;
  }

  peer_ptr->peer_connection->Join(transmission_id, password);
  LOG_INFO("JoinConnection[{}] with password [{}]", transmission_id, password);
  return 0;
}

int LeaveConnection(PeerPtr *peer_ptr) {
  if (!peer_ptr) {
    LOG_ERROR("peer_ptr not created");
    return -1;
  }

  peer_ptr->peer_connection->Leave();
  LOG_INFO("LeaveConnection");
  return 0;
}

int SendData(PeerPtr *peer_ptr, DATA_TYPE data_type, const char *data,
             size_t size) {
  if (!peer_ptr) {
    LOG_ERROR("peer_ptr not created");
    return -1;
  }

  if (DATA_TYPE::VIDEO == data_type) {
    peer_ptr->peer_connection->SendVideoData(data, size);
  } else if (DATA_TYPE::AUDIO == data_type) {
    peer_ptr->peer_connection->SendAudioData(data, size);
  } else if (DATA_TYPE::DATA == data_type) {
    peer_ptr->peer_connection->SendUserData(data, size);
  }
  return 0;
}