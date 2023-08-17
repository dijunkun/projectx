#include "ice_transmission.h"

#include <map>
#include <nlohmann/json.hpp>
#include <thread>

#include "log.h"

using nlohmann::json;

constexpr size_t HASH_STRING_PIECE(const char *string_piece) {
  std::size_t result = 0;
  while (*string_piece) {
    result = (result * 131) + *string_piece++;
  }
  return result;
}

constexpr size_t operator"" _H(const char *string_piece, size_t) {
  return HASH_STRING_PIECE(string_piece);
}

const std::vector<std::string> ice_status = {
    "JUICE_STATE_DISCONNECTED", "JUICE_STATE_GATHERING",
    "JUICE_STATE_CONNECTING",   "JUICE_STATE_CONNECTED",
    "JUICE_STATE_COMPLETED",    "JUICE_STATE_FAILED"};

IceTransmission::IceTransmission(
    WsTransmission *ice_ws_transmission,
    std::function<void(const char *, size_t)> on_receive_ice_msg)
    : ice_ws_transport_(ice_ws_transmission),
      on_receive_ice_msg_cb_(on_receive_ice_msg) {}

IceTransmission::~IceTransmission() {}

int IceTransmission::InitIceTransmission(std::string &ip, int port) {
  ice_agent_ = new IceAgent(ip, port);

  ice_agent_->CreateIceAgent(
      [](juice_agent_t *agent, juice_state_t state, void *user_ptr) {
        LOG_INFO("state_change: {}", ice_status[state]);
      },
      [](juice_agent_t *agent, const char *sdp, void *user_ptr) {
        LOG_INFO("candadite: {}", sdp);
        // trickle
        // static_cast<IceTransmission
        // *>(user_ptr)->SendOfferLocalCandidate(sdp);
      },
      [](juice_agent_t *agent, void *user_ptr) {
        LOG_INFO("gather_done");
        // non-trickle
        if (user_ptr) {
          static_cast<IceTransmission *>(user_ptr)->GetLocalSdp();
          static_cast<IceTransmission *>(user_ptr)->SendOffer();
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

int IceTransmission::InitIceTransmission(std::string &ip, int port,
                                         std::string const &id) {
  transmission_id_ = id;

  ice_agent_ = new IceAgent(ip, port);

  ice_agent_->CreateIceAgent(
      [](juice_agent_t *agent, juice_state_t state, void *user_ptr) {
        LOG_INFO("state_change: {}", ice_status[state]);
      },
      [](juice_agent_t *agent, const char *sdp, void *user_ptr) {
        LOG_INFO("candadite: {}", sdp);
        // trickle
        // static_cast<PeerConnection
        // *>(user_ptr)->SendAnswerLocalCandidate(sdp);
      },
      [](juice_agent_t *agent, void *user_ptr) {
        LOG_INFO("gather_done");
        // non-trickle
        if (user_ptr) {
          static_cast<IceTransmission *>(user_ptr)->CreateAnswer();
          static_cast<IceTransmission *>(user_ptr)->SendAnswer();
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
  if (ice_agent_) {
    delete ice_agent_;
  }
  return 0;
}

int IceTransmission::CreateTransmission(const std::string &transmission_id) {
  LOG_INFO("Create transport");
  offer_peer_ = true;

  // if (SignalStatus::Connected != signal_status_) {
  //   LOG_ERROR("Not connect to signalserver");
  //   return -1;
  // }

  json message = {{"type", "create_transmission"},
                  {"transmission_id", transmission_id}};
  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
    LOG_INFO("Send msg: {}", message.dump().c_str());
  }

  // CreateOffer();
  return 0;
}

int IceTransmission::JoinTransmission(const std::string &transmission_id) {
  LOG_INFO("Join transport");
  offer_peer_ = false;
  transmission_id_ = transmission_id;

  // if (SignalStatus::Connected != signal_status_) {
  //   LOG_ERROR("Not connect to signalserver");
  //   return -1;
  // }

  // QueryRemoteSdp(transmission_id);
  CreateOffer();
  return 0;
}

int IceTransmission::GatherCandidates() {
  ice_agent_->GatherCandidates();
  return 0;
}

int IceTransmission::GetLocalSdp() {
  local_sdp_ = ice_agent_->GenerateLocalSdp();
  return 0;
}

int IceTransmission::SetRemoteSdp(const std::string &remote_sdp) {
  ice_agent_->SetRemoteSdp(remote_sdp.c_str());
  return 0;
}

int IceTransmission::AddRemoteCandidate(const std::string &remote_candidate) {
  ice_agent_->AddRemoteCandidates(remote_candidate.c_str());
  return 0;
}

int IceTransmission::CreateOffer() {
  LOG_INFO("Create offer");
  GatherCandidates();
  return 0;
}

int IceTransmission::SendOffer() {
  json message = {{"type", "offer"},
                  {"transmission_id", transmission_id_},
                  {"sdp", local_sdp_}};
  LOG_INFO("Send offer:\n{}", message.dump().c_str());

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
  }
  return 0;
}

int IceTransmission::QueryRemoteSdp(std::string transmission_id) {
  json message = {{"type", "query_remote_sdp"},
                  {"transmission_id", transmission_id_}};
  LOG_INFO("Query remote sdp");

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
                  {"sdp", local_sdp_}};
  LOG_INFO("Send answer:\n{}", message.dump().c_str());

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
  }
  return 0;
}

int IceTransmission::SendOfferLocalCandidate(
    const std::string &remote_candidate) {
  json message = {{"type", "offer_candidate"},
                  {"transmission_id", transmission_id_},
                  {"sdp", remote_candidate}};
  LOG_INFO("Send candidate:\n{}", message.dump().c_str());

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
  LOG_INFO("Send candidate:\n{}", message.dump().c_str());

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
  auto j = json::parse(msg);
  LOG_INFO("msg: {}", msg.c_str());

  std::string type = j["type"];

  switch (HASH_STRING_PIECE(type.c_str())) {
    case "offer"_H: {
      break;
    }
    case "transmission_id"_H: {
      if (j["status"].get<std::string>() == "success") {
        transmission_id_ = j["transmission_id"].get<std::string>();
        LOG_INFO("Create transmission success with id [{}]", transmission_id_);
        // SendOffer();
      } else if (j["status"].get<std::string>() == "fail") {
        LOG_WARN("Create transmission failed with id [{}], due to [{}]",
                 transmission_id_, j["reason"].get<std::string>().c_str());
      }
      break;
    }
    case "remote_sdp"_H: {
      remote_sdp_ = j["sdp"].get<std::string>();

      if (remote_sdp_.empty()) {
        LOG_INFO("Offer peer not ready, wait 1 second and requery remote sdp");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        QueryRemoteSdp(transmission_id_);
      } else {
        LOG_INFO("Receive remote sdp [{}]", remote_sdp_);
        SetRemoteSdp(remote_sdp_);

        if (!offer_peer_) {
          GatherCandidates();
        }
      }
      break;
    }
    case "candidate"_H: {
      std::string candidate = j["sdp"].get<std::string>();
      LOG_INFO("Receive candidate [{}]", candidate);
      AddRemoteCandidate(candidate);
      break;
    }
    default:
      break;
  }
}
