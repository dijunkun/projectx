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
                           const std::string &transmission_id,
                           const std::string &user_id) {
  user_id_ = user_id;

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
          std::string transmission_id = j["transmission_id"].get<std::string>();
          std::string sdp = j["sdp"].get<std::string>();
          std::string remote_user_id = j["remote_user_id"].get<std::string>();
          LOG_INFO("Receive remote sdp from [{}]", remote_user_id);

          ice_transmission_list_[remote_user_id] = new IceTransmission(
              false, transmission_id, user_id_, remote_user_id, ws_transport_,
              on_receive_ice_msg_);

          ice_transmission_list_[remote_user_id]->InitIceTransmission(
              cfg_stun_server_ip_, stun_server_port_);

          ice_transmission_list_[remote_user_id]->SetTransmissionId(
              transmission_id_);

          ice_transmission_list_[remote_user_id]->SetRemoteSdp(remote_sdp);

          ice_transmission_list_[remote_user_id]->GatherCandidates();
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

  json message = {{"type", "create_transmission"},
                  {"user_id", user_id},
                  {"transmission_id", transmission_id}};
  if (ws_transport_) {
    ws_transport_->Send(message.dump());
    LOG_INFO("Send create transmission request: {}", message.dump().c_str());
  }
  return 0;
}

int PeerConnection::Join(PeerConnectionParams params,
                         const std::string &transmission_id,
                         const std::string &user_id) {
  // Todo: checkout user_id unique or not
  user_id_ = user_id;

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
    // do {
    // } while (ice_transmission_list_.empty());
    auto j = json::parse(msg);
    std::string type = j["type"];
    LOG_INFO("msg type :{}", type);
    switch (HASH_STRING_PIECE(type.c_str())) {
      case "transmission_members"_H: {
        transmission_member_list_ = j["transmission_members"];
        std::string transmission_id = j["transmission_id"];

        LOG_INFO("Transmission [{}] members: [", transmission_id);
        for (auto member : transmission_member_list_) {
          LOG_INFO("{}", member);
        }
        LOG_INFO("]");

        // if (transmission_member_list_.size() == 1 &&
        //     transmission_member_list_[0] == "host") {
        //   ice_transmission_list_["host"] = new IceTransmission(
        //       true, "host", ws_transport_, on_receive_ice_msg_);
        //   ice_transmission_list_["host"]->InitIceTransmission(
        //       cfg_stun_server_ip_, stun_server_port_);
        //   ice_transmission_list_["host"]->JoinTransmission(transmission_id,
        //                                                    user_id_);
        // } else {
        //   for (auto &member : transmission_member_list_) {
        //     ice_transmission_list_[member] = new IceTransmission(
        //         true, member, ws_transport_, on_receive_ice_msg_);
        //     ice_transmission_list_[member]->InitIceTransmission(
        //         cfg_stun_server_ip_, stun_server_port_);
        //     ice_transmission_list_[member]->JoinTransmission(transmission_id,
        //                                                      user_id_);
        //   }
        // }

        for (auto &remote_user_id : transmission_member_list_) {
          ice_transmission_list_[remote_user_id] = new IceTransmission(
              true, transmission_id, user_id_, remote_user_id, ws_transport_,
              on_receive_ice_msg_);
          ice_transmission_list_[remote_user_id]->InitIceTransmission(
              cfg_stun_server_ip_, stun_server_port_);
          ice_transmission_list_[remote_user_id]->JoinTransmission();
        }

        break;
      }
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
          std::string transmission_id = j["transmission_id"].get<std::string>();
          std::string sdp = j["sdp"].get<std::string>();
          std::string remote_user_id = j["remote_user_id"].get<std::string>();
          LOG_INFO("Receive remote sdp from [{}]", remote_user_id);

          ice_transmission_list_[remote_user_id] = new IceTransmission(
              false, transmission_id, user_id_, remote_user_id, ws_transport_,
              on_receive_ice_msg_);

          ice_transmission_list_[remote_user_id]->InitIceTransmission(
              cfg_stun_server_ip_, stun_server_port_);

          ice_transmission_list_[remote_user_id]->SetTransmissionId(
              transmission_id_);

          ice_transmission_list_[remote_user_id]->SetRemoteSdp(remote_sdp);

          ice_transmission_list_[remote_user_id]->GatherCandidates();
        }
        break;
      }
      case "remote_sdp"_H: {
        std::string remote_sdp = j["sdp"].get<std::string>();
        if (remote_sdp.empty()) {
          LOG_INFO("remote_sdp is empty");
        } else {
          std::string transmission_id = j["transmission_id"].get<std::string>();
          std::string sdp = j["sdp"].get<std::string>();
          std::string remote_user_id = j["remote_user_id"].get<std::string>();

          LOG_INFO("Receive remote sdp from [{}]", remote_user_id);
          // LOG_INFO("Receive remote sdp [{}]", remote_sdp);

          if (ice_transmission_list_.find(remote_user_id) !=
              ice_transmission_list_.end()) {
            ice_transmission_list_[remote_user_id]->SetRemoteSdp(remote_sdp);
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

  // ice_transmission_list_["self"] =
  //     new IceTransmission(true, ws_transport_, on_receive_ice_msg_);
  // ice_transmission_list_["self"]->InitIceTransmission(cfg_stun_server_ip_,
  //                                                     stun_server_port_);
  // ice_transmission_ =
  //     new IceTransmission(true, ws_transport_, on_receive_ice_msg_);
  // ice_transmission_->InitIceTransmission(cfg_stun_server_ip,
  // stun_server_port);

  do {
    // LOG_INFO("GetSignalStatus = {}", GetSignalStatus());
  } while (SignalStatus::Connected != GetSignalStatus());

  RequestTransmissionMemberList(transmission_id_);
  // ice_transmission_->JoinTransmission(transmission_id_);
  // ice_transmission_list_["self"]->JoinTransmission(transmission_id_);
  return 0;
}

int PeerConnection::RequestTransmissionMemberList(
    const std::string &transmission_id) {
  LOG_INFO("Request member list");

  json message = {{"type", "query_members"},
                  {"transmission_id", transmission_id_}};

  if (ws_transport_) {
    ws_transport_->Send(message.dump());
  }
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