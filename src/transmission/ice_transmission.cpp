#include "ice_transmission.h"

#include <map>
#include <nlohmann/json.hpp>
#include <thread>

#include "common.h"
#include "log.h"

using nlohmann::json;

const std::vector<std::string> ice_status = {
    "JUICE_STATE_DISCONNECTED", "JUICE_STATE_GATHERING",
    "JUICE_STATE_CONNECTING",   "JUICE_STATE_CONNECTED",
    "JUICE_STATE_COMPLETED",    "JUICE_STATE_FAILED"};

IceTransmission::IceTransmission(
    bool offer_peer, std::string &transmission_id, std::string &user_id,
    std::string &remote_user_id, WsTransmission *ice_ws_transmission,
    std::function<void(const char *, size_t)> on_receive_ice_msg)
    : offer_peer_(offer_peer),
      transmission_id_(transmission_id),
      user_id_(user_id),
      remote_user_id_(remote_user_id),
      ice_ws_transport_(ice_ws_transmission),
      on_receive_ice_msg_cb_(on_receive_ice_msg) {}

IceTransmission::~IceTransmission() {
  if (ice_agent_) {
    delete ice_agent_;
    ice_agent_ = nullptr;
  }
}

int IceTransmission::InitIceTransmission(std::string &ip, int port) {
  ice_agent_ = new IceAgent(ip, port);

  ice_agent_->CreateIceAgent(
      [](juice_agent_t *agent, juice_state_t state, void *user_ptr) {
        if (user_ptr) {
          IceTransmission *ice_transmission_obj =
              static_cast<IceTransmission *>(user_ptr);
          LOG_INFO("[{}->{}] state_change: {}", ice_transmission_obj->user_id_,
                   ice_transmission_obj->remote_user_id_, ice_status[state]);
        } else {
          LOG_INFO("state_change: {}", ice_status[state]);
        }
      },
      [](juice_agent_t *agent, const char *sdp, void *user_ptr) {
        // LOG_INFO("candadite: {}", sdp);
        // trickle
        // static_cast<IceTransmission
        // *>(user_ptr)->SendOfferLocalCandidate(sdp);
      },
      [](juice_agent_t *agent, void *user_ptr) {
        // non-trickle
        if (user_ptr) {
          IceTransmission *ice_transmission_obj =
              static_cast<IceTransmission *>(user_ptr);
          LOG_INFO("[{}] gather_done", ice_transmission_obj->user_id_);

          if (ice_transmission_obj->offer_peer_) {
            ice_transmission_obj->GetLocalSdp();
            ice_transmission_obj->SendOffer();
          } else {
            ice_transmission_obj->CreateAnswer();
            ice_transmission_obj->SendAnswer();
          }
        }
      },
      [](juice_agent_t *agent, const char *data, size_t size, void *user_ptr) {
        if (user_ptr &&
            static_cast<IceTransmission *>(user_ptr)->on_receive_ice_msg_cb_) {
          static_cast<IceTransmission *>(user_ptr)->on_receive_ice_msg_cb_(
              data, size);
        }
      },
      this);
  return 0;
}

int IceTransmission::DestroyIceTransmission() {
  LOG_INFO("[{}->{}] Destroy ice transmission", user_id_, remote_user_id_);
  return ice_agent_->DestoryIceAgent();
}

int IceTransmission::CreateTransmission(const std::string &transmission_id) {
  LOG_INFO("[{}] Create transmission", user_id_);
  offer_peer_ = false;
  transmission_id_ = transmission_id;

  json message = {{"type", "create_transmission"},
                  {"transmission_id", transmission_id}};
  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
    LOG_INFO("Send msg: {}", message.dump().c_str());
  }

  return 0;
}

int IceTransmission::SetTransmissionId(const std::string &transmission_id) {
  transmission_id_ = transmission_id;

  return 0;
}

int IceTransmission::JoinTransmission() {
  LOG_INFO("[{}] Join transmission", user_id_);

  CreateOffer();
  return 0;
}

int IceTransmission::GatherCandidates() {
  ice_agent_->GatherCandidates();
  LOG_INFO("[{}] Gather candidates", user_id_);
  return 0;
}

int IceTransmission::GetLocalSdp() {
  local_sdp_ = ice_agent_->GenerateLocalSdp();
  LOG_INFO("[{}] generate local sdp", user_id_);
  return 0;
}

int IceTransmission::SetRemoteSdp(const std::string &remote_sdp) {
  ice_agent_->SetRemoteSdp(remote_sdp.c_str());
  LOG_INFO("[{}] set remote sdp", user_id_);
  remote_ice_username_ = GetIceUsername(remote_sdp);
  return 0;
}

int IceTransmission::AddRemoteCandidate(const std::string &remote_candidate) {
  ice_agent_->AddRemoteCandidates(remote_candidate.c_str());
  return 0;
}

int IceTransmission::CreateOffer() {
  LOG_INFO("[{}] create offer", user_id_);
  GatherCandidates();
  return 0;
}

int IceTransmission::SendOffer() {
  json message = {{"type", "offer"},
                  {"transmission_id", transmission_id_},
                  {"user_id", user_id_},
                  {"remote_user_id", remote_user_id_},
                  {"sdp", local_sdp_}};
  // LOG_INFO("Send offer:\n{}", message.dump());

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
    LOG_INFO("[{}->{}] send offer", user_id_, remote_user_id_);
  }
  return 0;
}

int IceTransmission::QueryRemoteSdp(std::string transmission_id) {
  json message = {{"type", "query_remote_sdp"},
                  {"transmission_id", transmission_id_}};
  LOG_INFO("[{}] query remote sdp", user_id_);

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
  }
  return 0;
}

int IceTransmission::CreateAnswer() {
  GetLocalSdp();
  return 0;
}

int IceTransmission::SendAnswer() {
  json message = {{"type", "answer"},
                  {"transmission_id", transmission_id_},
                  {"sdp", local_sdp_},
                  {"user_id", user_id_},
                  {"remote_user_id", remote_user_id_}};

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
    LOG_INFO("[{}->{}] send answer", user_id_, remote_user_id_);
  }
  return 0;
}

int IceTransmission::SendOfferLocalCandidate(
    const std::string &remote_candidate) {
  json message = {{"type", "offer_candidate"},
                  {"transmission_id", transmission_id_},
                  {"sdp", remote_candidate}};
  // LOG_INFO("Send candidate:\n{}", message.dump().c_str());
  LOG_INFO("[{}] send candidate", user_id_);

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
  }
  return 0;
}

int IceTransmission::SendAnswerLocalCandidate(
    const std::string &remote_candidate) {
  json message = {{"type", "answer_candidate"},
                  {"transmission_id", transmission_id_},
                  {"sdp", remote_candidate}};
  // LOG_INFO("Send candidate:\n{}", message.dump().c_str());
  LOG_INFO("[{}] send candidate", user_id_);

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
  }
  return 0;
}

int IceTransmission::SendData(const char *data, size_t size) {
  ice_agent_->Send(data, size);
  return 0;
}

void IceTransmission::OnReceiveMessage(const std::string &msg) {
  // auto j = json::parse(msg);
  // LOG_INFO("msg: {}", msg.c_str());

  // std::string type = j["type"];

  // switch (HASH_STRING_PIECE(type.c_str())) {
  //   case "offer"_H: {
  //     remote_sdp_ = j["sdp"].get<std::string>();
  //     break;
  //   }
  //   default:
  //     break;
  // }
}
