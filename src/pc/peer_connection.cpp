#include "peer_connection.h"

#include <regex>

#include "INIReader.h"
#include "log.h"
#include "nlohmann/json.hpp"

using nlohmann::json;

static const std::map<std::string, unsigned int> siganl_types{
    {"connection_id", 1},
    {"offer", 2},
    {"transport_id", 3},
    {"remote_sdp", 4},
    {"candidate", 5}};

PeerConnection::PeerConnection() {}

PeerConnection::~PeerConnection() {}

int PeerConnection::Init(PeerConnectionParams params) {
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

  on_receive_ice_msg_ = [this](const char *data, size_t size) {
    std::string msg(data, size);
    LOG_INFO("Receive data: [{}]", msg.c_str());
  };

  ws_transport_ = new WsTransport(on_receive_ws_msg_);
  uri_ = "ws://" + cfg_signal_server_ip + ":" + cfg_signal_server_port;
  if (ws_transport_) {
    ws_transport_->Connect(uri_);
  }

  ice_transport_ = new IceTransport(ws_transport_, on_receive_ice_msg_);
  ice_transport_->InitIceTransport(cfg_stun_server_ip, stun_server_port);

  do {
    LOG_INFO("GetSignalStatus = {}", GetSignalStatus());
  } while (SignalStatus::Connected != GetSignalStatus());

  ice_transport_->CreateTransport();
  return 0;
}

int PeerConnection::Init(PeerConnectionParams params, std::string const &id) {
  INIReader reader(params.cfg_path);
  std::string cfg_signal_server_ip = reader.Get("signal server", "ip", "-1");
  std::string cfg_signal_server_port =
      reader.Get("signal server", "port", "-1");
  std::string cfg_stun_server_ip = reader.Get("stun server", "ip", "-1");
  std::string cfg_stun_server_port = reader.Get("stun server", "port", "-1");
  std::regex regex("\n");

  int signal_server_port = stoi(cfg_signal_server_port);
  int stun_server_port = stoi(cfg_signal_server_port);

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

  on_receive_ice_msg_ = [this](const char *data, size_t size) {
    std::string msg(data, size);
    LOG_INFO("Receive data: [{}]", msg.c_str());
  };

  transport_id_ = id;

  ws_transport_ = new WsTransport(on_receive_ws_msg_);
  uri_ = "ws://" + cfg_signal_server_ip + ":" + cfg_signal_server_port;
  if (ws_transport_) {
    ws_transport_->Connect(uri_);
  }

  ice_transport_ = new IceTransport(ws_transport_, on_receive_ice_msg_);
  ice_transport_->InitIceTransport(cfg_stun_server_ip, stun_server_port, id);

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

int PeerConnection::SendData(const char *data, size_t size) {
  ice_transport_->SendData(data, size);
  return 0;
}