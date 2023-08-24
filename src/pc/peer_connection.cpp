#include "peer_connection.h"

#include <regex>

#include "INIReader.h"
#include "common.h"
#include "log.h"
#include "nlohmann/json.hpp"

using nlohmann::json;

PeerConnection::PeerConnection() {}

PeerConnection::~PeerConnection() {}

int PeerConnection::Init(PeerConnectionParams params,
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

  LOG_INFO("stun server ip [{}] port [{}]", cfg_stun_server_ip_,
           stun_server_port_);

  on_receive_ws_msg_ = [this](const std::string &msg) { ProcessSignal(msg); };

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
  } while (SignalStatus::Connected != GetSignalStatus());

  return 0;
}

int PeerConnection::Create(PeerConnectionParams params,
                           const std::string &transmission_id,
                           const std::string &user_id) {
  int ret = 0;

  ret = Init(params, transmission_id, user_id);

  json message = {{"type", "create_transmission"},
                  {"user_id", user_id},
                  {"transmission_id", transmission_id}};
  if (ws_transport_) {
    ws_transport_->Send(message.dump());
    LOG_INFO("Send create transmission request, transmission_id [{}]",
             transmission_id);
  }
  return ret;
}

int PeerConnection::Join(PeerConnectionParams params,
                         const std::string &transmission_id,
                         const std::string &user_id) {
  int ret = 0;

  ret = Init(params, transmission_id, user_id);

  transmission_id_ = transmission_id;
  ret = RequestTransmissionMemberList(transmission_id_);
  return ret;
}

void PeerConnection::ProcessSignal(const std::string &signal) {
  auto j = json::parse(signal);
  std::string type = j["type"];
  LOG_INFO("signal type: {}", type);
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
        LOG_INFO("Create transmission success with id [{}]", transmission_id_);

      } else if (j["status"].get<std::string>() == "fail") {
        LOG_WARN("Create transmission failed with id [{}], due to [{}]",
                 transmission_id_, j["reason"].get<std::string>().c_str());
      }
      break;
    }
    case "user_id_list"_H: {
      user_id_list_ = j["user_id_list"];
      std::string transmission_id = j["transmission_id"];

      LOG_INFO("Transmission [{}] members: [", transmission_id);
      for (auto user_id : user_id_list_) {
        LOG_INFO("{}", user_id);
      }
      LOG_INFO("]");

      for (auto &remote_user_id : user_id_list_) {
        ice_transmission_list_[remote_user_id] =
            new IceTransmission(true, transmission_id, user_id_, remote_user_id,
                                ws_transport_, on_receive_ice_msg_);
        ice_transmission_list_[remote_user_id]->InitIceTransmission(
            cfg_stun_server_ip_, stun_server_port_);
        ice_transmission_list_[remote_user_id]->JoinTransmission();
      }

      break;
    }
    case "user_leave_transmission"_H: {
      std::string user_id = j["user_id"];
      LOG_INFO("Receive notification: user id [{}] leave transmission",
               user_id);
      auto user_id_it = ice_transmission_list_.find(user_id);
      if (user_id_it != ice_transmission_list_.end()) {
        user_id_it->second->DestroyIceTransmission();
        delete user_id_it->second;
        user_id_it->second = nullptr;
        ice_transmission_list_.erase(user_id_it);
        LOG_INFO("Terminate transmission to user [{}]", user_id);
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
        LOG_INFO("[{}] receive offer from [{}]", user_id_, remote_user_id);

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
    case "answer"_H: {
      std::string remote_sdp = j["sdp"].get<std::string>();
      if (remote_sdp.empty()) {
        LOG_INFO("remote_sdp is empty");
      } else {
        std::string transmission_id = j["transmission_id"].get<std::string>();
        std::string sdp = j["sdp"].get<std::string>();
        std::string remote_user_id = j["remote_user_id"].get<std::string>();

        LOG_INFO("[{}] receive answer from [{}]", user_id_, remote_user_id);

        if (ice_transmission_list_.find(remote_user_id) !=
            ice_transmission_list_.end()) {
          ice_transmission_list_[remote_user_id]->SetRemoteSdp(remote_sdp);
        }
      }
      break;
    }
    default: {
      // ice_transmission_->OnReceiveMessage(msg);
      break;
    }
  }
}

int PeerConnection::RequestTransmissionMemberList(
    const std::string &transmission_id) {
  LOG_INFO("Request member list");

  json message = {{"type", "query_user_id_list"},
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