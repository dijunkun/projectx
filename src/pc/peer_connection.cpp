#include "peer_connection.h"

#include <regex>

#include "INIReader.h"
#include "log.h"
#include "nlohmann/json.hpp"

using nlohmann::json;

static const std::map<std::string, unsigned int> siganl_types{
    {"ws_connection_id", 1},
    {"offer", 2},
    {"transmission_id", 3},
    {"remote_sdp", 4},
    {"candidate", 5}};

PeerConnection::PeerConnection() {}

PeerConnection::~PeerConnection() {}

int PeerConnection::Create(PeerConnectionParams params, const std::string &id) {
  INIReader reader(params.cfg_path);
  std::string cfg_signal_server_ip = reader.Get("signal server", "ip", "-1");
  std::string cfg_signal_server_port =
      reader.Get("signal server", "port", "-1");
  std::string cfg_stun_server_ip = reader.Get("stun server", "ip", "-1");
  std::string cfg_stun_server_port = reader.Get("stun server", "port", "-1");
  std::regex regex("\n");

  LOG_INFO("Read config success");

  int signal_server_port = stoi(cfg_signal_server_port);
  int stun_server_port = stoi(cfg_stun_server_port);

  on_receive_ws_msg_ = [this](const std::string &msg) {
    do {
    } while (!ice_transmission_);
    auto j = json::parse(msg);
    std::string type = j["type"];
    auto itr = siganl_types.find(type);
    if (itr != siganl_types.end()) {
      LOG_INFO("msg type :{}", itr->first);
      switch (itr->second) {
        case 1: {
          ws_connection_id_ = j["ws_connection_id"].get<unsigned int>();
          LOG_INFO("Receive local peer websocket connection id [{}]",
                   ws_connection_id_);
          signal_status_ = SignalStatus::Connected;
          break;
        }
        default: {
          ice_transmission_->OnReceiveMessage(msg);
          break;
        }
      }
    }
  };

  on_receive_ice_msg_ = [this](const char *data, size_t size) {
    std::string msg(data, size);
    LOG_INFO("Receive data: [{}]", msg.c_str());
  };

  ws_transport_ = new WsTransmission(on_receive_ws_msg_);
  uri_ = "ws://" + cfg_signal_server_ip + ":" + cfg_signal_server_port;
  if (ws_transport_) {
    ws_transport_->Connect(uri_);
  }

  ice_transmission_ = new IceTransmission(ws_transport_, on_receive_ice_msg_);
  ice_transmission_->InitIceTransmission(cfg_stun_server_ip, stun_server_port);

  do {
    LOG_INFO("GetSignalStatus = {}", GetSignalStatus());
  } while (SignalStatus::Connected != GetSignalStatus());

  ice_transmission_->CreateTransmission(id);
  return 0;
}

int PeerConnection::Join(PeerConnectionParams params, const std::string &id) {
  INIReader reader(params.cfg_path);
  std::string cfg_signal_server_ip = reader.Get("signal server", "ip", "-1");
  std::string cfg_signal_server_port =
      reader.Get("signal server", "port", "-1");
  std::string cfg_stun_server_ip = reader.Get("stun server", "ip", "-1");
  std::string cfg_stun_server_port = reader.Get("stun server", "port", "-1");
  std::regex regex("\n");

  int signal_server_port = stoi(cfg_signal_server_port);
  int stun_server_port = stoi(cfg_stun_server_port);

  on_receive_ws_msg_ = [this](const std::string &msg) {
    do {
    } while (!ice_transmission_);
    auto j = json::parse(msg);
    std::string type = j["type"];
    auto itr = siganl_types.find(type);
    if (itr != siganl_types.end()) {
      LOG_INFO("msg type :{}", itr->first);
      switch (itr->second) {
        case 1: {
          ws_connection_id_ = j["ws_connection_id"].get<unsigned int>();
          LOG_INFO("Receive local peer websocket connection id [{}]",
                   ws_connection_id_);
          signal_status_ = SignalStatus::Connected;
          break;
        }
        default: {
          ice_transmission_->OnReceiveMessage(msg);
          break;
        }
      }
    }
  };

  on_receive_ice_msg_ = [this](const char *data, size_t size) {
    std::string msg(data, size);
    LOG_INFO("Receive data: [{}]", msg.c_str());
  };

  transmission_id_ = id;

  ws_transport_ = new WsTransmission(on_receive_ws_msg_);
  uri_ = "ws://" + cfg_signal_server_ip + ":" + cfg_signal_server_port;
  if (ws_transport_) {
    ws_transport_->Connect(uri_);
  }

  ice_transmission_ = new IceTransmission(ws_transport_, on_receive_ice_msg_);
  ice_transmission_->InitIceTransmission(cfg_stun_server_ip, stun_server_port,
                                         id);

  do {
    LOG_INFO("GetSignalStatus = {}", GetSignalStatus());
  } while (SignalStatus::Connected != GetSignalStatus());

  ice_transmission_->JoinTransmission(transmission_id_);
  return 0;
}

int PeerConnection::Destroy() {
  if (ws_transport_) {
    delete ws_transport_;
  }
  return 0;
}

SignalStatus PeerConnection::GetSignalStatus() { return signal_status_; }

int PeerConnection::SendData(const char *data, size_t size) {
  ice_transmission_->SendData(data, size);
  return 0;
}