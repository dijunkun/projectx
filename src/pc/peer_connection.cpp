#include "peer_connection.h"

#include <regex>

#include "INIReader.h"
#include "common.h"
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

int PeerConnection::Create(PeerConnectionParams params,
                           const std::string &transmission_id) {
  INIReader reader(params.cfg_path);
  cfg_signal_server_ip_ = reader.Get("signal server", "ip", "-1");
  cfg_signal_server_port_ = reader.Get("signal server", "port", "-1");
  cfg_stun_server_ip_ = reader.Get("stun server", "ip", "-1");
  cfg_stun_server_port_ = reader.Get("stun server", "port", "-1");
  std::regex regex("\n");

  LOG_INFO("Read config success");

  signal_server_port_ = stoi(cfg_signal_server_port_);
  stun_server_port_ = stoi(cfg_stun_server_port_);

  on_receive_ws_msg_ = [this](const std::string &msg) {
    auto j = json::parse(msg);
    std::string type = j["type"];
    LOG_INFO("msg type :{}", type.c_str());
    switch (HASH_STRING_PIECE(type.c_str())) {
      case "ws_connection_id"_H: {
        ws_connection_id_ = j["ws_connection_id"].get<unsigned int>();
        LOG_INFO("Receive local peer websocket connection id [{}]",
                 ws_connection_id_);
        signal_status_ = SignalStatus::Connected;
        break;
      }
      case "transmission_id"_H: {
        if (j["status"].get<std::string>() == "success") {
          transmission_id_ = j["transmission_id"].get<std::string>();
          LOG_INFO("Create transmission success with id [{}]",
                   transmission_id_);

        } else if (j["status"].get<std::string>() == "fail") {
          LOG_WARN("Create transmission failed with id [{}], due to [{}]",
                   transmission_id_, j["reason"].get<std::string>().c_str());
        }
        break;
      }
      case "offer"_H: {
        std::string remote_sdp = j["sdp"].get<std::string>();

        if (remote_sdp.empty()) {
          LOG_INFO("Invalid remote sdp");
        } else {
          std::string ice_username = GetIceUsername(remote_sdp);
          LOG_INFO("Receive remote sdp from [{}]", ice_username);

          // IceTransmission *ice_transmission =
          //     new IceTransmission(false, ws_transport_, on_receive_ice_msg_);

          ice_transmission_list_[ice_username] =
              new IceTransmission(false, ws_transport_, on_receive_ice_msg_);
          ice_transmission_list_[ice_username]->InitIceTransmission(
              cfg_stun_server_ip_, stun_server_port_);

          ice_transmission_list_[ice_username]->SetRemoteSdp(remote_sdp);

          ice_transmission_list_[ice_username]->GatherCandidates();
        }
        break;
      }
      default: {
        // ice_transmission_->OnReceiveMessage(msg);
        break;
      }
    }
  };

  on_receive_ice_msg_ = [this](const char *data, size_t size) {
    std::string msg(data, size);
    LOG_INFO("Receive data: [{}]", msg.c_str());
  };

  ws_transport_ = new WsTransmission(on_receive_ws_msg_);
  uri_ = "ws://" + cfg_signal_server_ip_ + ":" + cfg_signal_server_port_;
  if (ws_transport_) {
    ws_transport_->Connect(uri_);
  }

  do {
    LOG_INFO("GetSignalStatus = {}", GetSignalStatus());
  } while (SignalStatus::Connected != GetSignalStatus());

  // ice_transmission_->CreateTransmission(transmission_id);

  json message = {{"type", "create_transmission"},
                  {"transmission_id", transmission_id}};
  if (ws_transport_) {
    ws_transport_->Send(message.dump());
    LOG_INFO("Send create transmission request: {}", message.dump().c_str());
  }
  return 0;
}

int PeerConnection::Join(PeerConnectionParams params,
                         const std::string &transmission_id) {
  INIReader reader(params.cfg_path);
  cfg_signal_server_ip_ = reader.Get("signal server", "ip", "-1");
  cfg_signal_server_port_ = reader.Get("signal server", "port", "-1");
  cfg_stun_server_ip_ = reader.Get("stun server", "ip", "-1");
  cfg_stun_server_port_ = reader.Get("stun server", "port", "-1");
  std::regex regex("\n");

  LOG_INFO("Read config success");

  signal_server_port_ = stoi(cfg_signal_server_port_);
  stun_server_port_ = stoi(cfg_stun_server_port_);

  on_receive_ws_msg_ = [this](const std::string &msg) {
    do {
    } while (ice_transmission_list_.empty());
    auto j = json::parse(msg);
    std::string type = j["type"];
    LOG_INFO("msg type :{}", type.c_str());
    switch (HASH_STRING_PIECE(type.c_str())) {
      case "ws_connection_id"_H: {
        ws_connection_id_ = j["ws_connection_id"].get<unsigned int>();
        LOG_INFO("Receive local peer websocket connection id [{}]",
                 ws_connection_id_);
        signal_status_ = SignalStatus::Connected;
        break;
      }
      case "offer"_H: {
        std::string remote_sdp = j["sdp"].get<std::string>();

        if (remote_sdp.empty()) {
          LOG_INFO("Invalid remote sdp");
        } else {
          std::string ice_username = GetIceUsername(remote_sdp);
          LOG_INFO("Receive remote sdp from [{}]", ice_username);

          // IceTransmission *ice_transmission =
          //     new IceTransmission(false, ws_transport_, on_receive_ice_msg_);

          ice_transmission_list_[ice_username] =
              new IceTransmission(false, ws_transport_, on_receive_ice_msg_);
          ice_transmission_list_[ice_username]->InitIceTransmission(
              cfg_stun_server_ip_, stun_server_port_);

          ice_transmission_list_[ice_username]->SetRemoteSdp(remote_sdp);

          ice_transmission_list_[ice_username]->GatherCandidates();
        }
        break;
      }
      case "remote_sdp"_H: {
        std::string remote_sdp = j["sdp"].get<std::string>();
        if (remote_sdp.empty()) {
          LOG_INFO("remote_sdp is empty");
        } else {
          std::string ice_username = GetIceUsername(remote_sdp);
          LOG_INFO("Receive remote sdp from [{}]", ice_username);
          // LOG_INFO("Receive remote sdp [{}]", remote_sdp);

          if (ice_transmission_list_.size() == 1 &&
              ice_transmission_list_.begin()->first == "self") {
            ice_transmission_list_["self"]->SetRemoteSdp(remote_sdp);
          } else if (ice_transmission_list_.find(ice_username) ==
                     ice_transmission_list_.end()) {
            ice_transmission_list_[ice_username] =
                new IceTransmission(false, ws_transport_, on_receive_ice_msg_);
            ice_transmission_list_[ice_username]->InitIceTransmission(
                cfg_stun_server_ip_, stun_server_port_);
            ice_transmission_list_[ice_username]->SetRemoteSdp(remote_sdp);
          }

          // if (!offer_peer_) {
          //   GatherCandidates();
          // }
        }
        break;
      }
      case "candidate"_H: {
        std::string remote_sdp_with_candidates = j["sdp"].get<std::string>();
        std::string ice_username = GetIceUsername(remote_sdp_with_candidates);
        LOG_INFO("Receive remote candidates from [{}]", ice_username);
        // LOG_INFO("Receive candidate [{}]", candidate);

        ice_transmission_list_[ice_username]->AddRemoteCandidate(
            remote_sdp_with_candidates);
        break;
      }
      default: {
        ice_transmission_->OnReceiveMessage(msg);
        break;
      }
    }
  };

  on_receive_ice_msg_ = [this](const char *data, size_t size) {
    std::string msg(data, size);
    LOG_INFO("Receive data: [{}]", msg.c_str());
  };

  transmission_id_ = transmission_id;

  ws_transport_ = new WsTransmission(on_receive_ws_msg_);
  uri_ = "ws://" + cfg_signal_server_ip_ + ":" + cfg_signal_server_port_;
  if (ws_transport_) {
    ws_transport_->Connect(uri_);
  }

  ice_transmission_list_["self"] =
      new IceTransmission(true, ws_transport_, on_receive_ice_msg_);
  ice_transmission_list_["self"]->InitIceTransmission(cfg_stun_server_ip_,
                                                      stun_server_port_);
  // ice_transmission_ =
  //     new IceTransmission(true, ws_transport_, on_receive_ice_msg_);
  // ice_transmission_->InitIceTransmission(cfg_stun_server_ip,
  // stun_server_port);

  do {
    // LOG_INFO("GetSignalStatus = {}", GetSignalStatus());
  } while (SignalStatus::Connected != GetSignalStatus());

  // ice_transmission_->JoinTransmission(transmission_id_);
  ice_transmission_list_["self"]->JoinTransmission(transmission_id_);
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
  for (auto ice_trans : ice_transmission_list_) {
    ice_trans.second->SendData(data, size);
  }
  return 0;
}