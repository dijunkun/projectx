#include "peer_connection.h"

#include <nlohmann/json.hpp>

#include "log.h"

using nlohmann::json;

static const std::map<std::string, unsigned int> siganl_types{
    {"connection_id", 1},
    {"offer", 2},
    {"transport_id", 3},
    {"remote_sdp", 4},
    {"candidate", 5}};

PeerConnection::PeerConnection() {}

PeerConnection::~PeerConnection() {}

int PeerConnection::Init(std::string const &uri) {
  on_receive_ws_msg_ = [this](const std::string &msg) {
    do {
    } while (!ice_transport_);
    auto j = json::parse(msg);
    std::string type = j["type"];
    auto itr = siganl_types.find(type);
    if (itr != siganl_types.end()) {
      LOG_INFO("msg type :{}", itr->first);
      switch (itr->second) {
        case 1: {
          connection_id_ = j["connection_id"].get<unsigned int>();
          LOG_INFO("Receive local peer connection_id [{}]", connection_id_);
          signal_status_ = SignalStatus::Connected;
          break;
        }
        default: {
          ice_transport_->OnReceiveMessage(msg);
          break;
        }
      }
    }
  };

  on_receive_ice_msg_ = [this](const char *data, size_t size) {};

  ws_transport_ = new WsTransport(on_receive_ws_msg_);
  if (ws_transport_) {
    ws_transport_->Connect(uri);
  }

  ice_transport_ = new IceTransport(ws_transport_, on_receive_ice_msg_);
  ice_transport_->InitIceTransport();

  do {
    LOG_INFO("GetSignalStatus = {}", GetSignalStatus());
  } while (SignalStatus::Connected != GetSignalStatus());

  ice_transport_->CreateTransport();
  return 0;
}

int PeerConnection::Init(std::string const &uri, std::string const &id) {
  on_receive_ws_msg_ = [this](const std::string &msg) {
    do {
    } while (!ice_transport_);
    auto j = json::parse(msg);
    std::string type = j["type"];
    auto itr = siganl_types.find(type);
    if (itr != siganl_types.end()) {
      LOG_INFO("msg type :{}", itr->first);
      switch (itr->second) {
        case 1: {
          connection_id_ = j["connection_id"].get<unsigned int>();
          LOG_INFO("Receive local peer connection_id [{}]", connection_id_);
          signal_status_ = SignalStatus::Connected;
          break;
        }
        default: {
          ice_transport_->OnReceiveMessage(msg);
          break;
        }
      }
    }
  };

  on_receive_ice_msg_ = [this](const char *data, size_t size) {};

  transport_id_ = id;

  ws_transport_ = new WsTransport(on_receive_ws_msg_);
  if (ws_transport_) {
    ws_transport_->Connect(uri);
  }

  ice_transport_ = new IceTransport(ws_transport_, on_receive_ice_msg_);
  ice_transport_->InitIceTransport(id);

  do {
    LOG_INFO("GetSignalStatus = {}", GetSignalStatus());
  } while (SignalStatus::Connected != GetSignalStatus());

  ice_transport_->CreateTransport(transport_id_);
  return 0;
}

int PeerConnection::Destroy() {
  if (ws_transport_) {
    delete ws_transport_;
  }
  return 0;
}

SignalStatus PeerConnection::GetSignalStatus() { return signal_status_; }