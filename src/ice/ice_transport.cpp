#include "ice_transport.h"

#include <map>
#include <nlohmann/json.hpp>

#include "log.h"

using nlohmann::json;

static const std::map<std::string, unsigned int> siganl_types{
    {"connection_id", 1},
    {"offer", 2},
    {"transport_id", 3},
    {"remote_sdp", 4},
    {"candidate", 5}};

const std::vector<std::string> ice_status = {
    "JUICE_STATE_DISCONNECTED", "JUICE_STATE_GATHERING",
    "JUICE_STATE_CONNECTING",   "JUICE_STATE_CONNECTED",
    "JUICE_STATE_COMPLETED",    "JUICE_STATE_FAILED"};

IceTransport::IceTransport(
    WsTransport *ice_ws_transport,
    std::function<void(const char *, size_t)> on_receive_ice_msg)
    : ice_ws_transport_(ice_ws_transport),
      on_receive_ice_msg_cb_(on_receive_ice_msg) {}

IceTransport::~IceTransport() {}

int IceTransport::InitIceTransport(std::string &ip, int port) {
  ice_agent_ = new IceAgent(ip, port);

  ice_agent_->CreateIceAgent(
      [](juice_agent_t *agent, juice_state_t state, void *user_ptr) {
        LOG_INFO("state_change: {}", ice_status[state]);
      },
      [](juice_agent_t *agent, const char *sdp, void *user_ptr) {
        LOG_INFO("candadite: {}", sdp);
        // trickle
        // static_cast<IceTransport
        // *>(user_ptr)->SendOfferLocalCandidate(sdp);
      },
      [](juice_agent_t *agent, void *user_ptr) {
        LOG_INFO("gather_done");
        // non-trickle
        if (user_ptr) {
          static_cast<IceTransport *>(user_ptr)->GetLocalSdp();
          static_cast<IceTransport *>(user_ptr)->SendOffer();
        }
      },
      [](juice_agent_t *agent, const char *data, size_t size, void *user_ptr) {
        if (user_ptr &&
            static_cast<IceTransport *>(user_ptr)->on_receive_ice_msg_cb_) {
          static_cast<IceTransport *>(user_ptr)->on_receive_ice_msg_cb_(data,
                                                                        size);
        }
      },
      this);
  return 0;
}

int IceTransport::InitIceTransport(std::string &ip, int port,
                                   std::string const &id) {
  transport_id_ = id;

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
          static_cast<IceTransport *>(user_ptr)->CreateAnswer();
          static_cast<IceTransport *>(user_ptr)->SendAnswer();
        }
      },
      [](juice_agent_t *agent, const char *data, size_t size, void *user_ptr) {
        if (user_ptr &&
            static_cast<IceTransport *>(user_ptr)->on_receive_ice_msg_cb_) {
          static_cast<IceTransport *>(user_ptr)->on_receive_ice_msg_cb_(data,
                                                                        size);
        }
      },
      this);
  return 0;
}

int IceTransport::DestroyIceTransport() {
  if (ice_agent_) {
    delete ice_agent_;
  }
  return 0;
}

int IceTransport::CreateTransport() {
  LOG_INFO("Create transport");
  offer_peer_ = true;

  // if (SignalStatus::Connected != signal_status_) {
  //   LOG_ERROR("Not connect to signalserver");
  //   return -1;
  // }

  json message = {{"type", "create_transport"}};
  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
    LOG_INFO("Send msg: {}", message.dump().c_str());
  }

  CreateOffer();
  return 0;
}

int IceTransport::CreateTransport(std::string transport_id) {
  LOG_INFO("Join transport");
  offer_peer_ = false;

  // if (SignalStatus::Connected != signal_status_) {
  //   LOG_ERROR("Not connect to signalserver");
  //   return -1;
  // }

  QueryRemoteSdp(transport_id);
  return 0;
}

int IceTransport::GatherCandidates() {
  ice_agent_->GatherCandidates();
  return 0;
}

int IceTransport::GetLocalSdp() {
  local_sdp_ = ice_agent_->GenerateLocalSdp();
  return 0;
}

int IceTransport::SetRemoteSdp(const std::string &remote_sdp) {
  ice_agent_->SetRemoteSdp(remote_sdp.c_str());
  return 0;
}

int IceTransport::AddRemoteCandidate(const std::string &remote_candidate) {
  ice_agent_->AddRemoteCandidates(remote_candidate.c_str());
  return 0;
}

int IceTransport::CreateOffer() {
  LOG_INFO("Create offer");
  GatherCandidates();
  return 0;
}

int IceTransport::SendOffer() {
  json message = {
      {"type", "offer"}, {"transport_id", transport_id_}, {"sdp", local_sdp_}};
  LOG_INFO("Send offer:\n{}", message.dump().c_str());

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
  }
  return 0;
}

int IceTransport::QueryRemoteSdp(std::string transport_id) {
  json message = {{"type", "query_remote_sdp"},
                  {"transport_id", transport_id_}};
  LOG_INFO("Query remote sdp");

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
  }
  return 0;
}

int IceTransport::CreateAnswer() {
  GetLocalSdp();
  return 0;
}

int IceTransport::SendAnswer() {
  json message = {
      {"type", "answer"}, {"transport_id", transport_id_}, {"sdp", local_sdp_}};
  LOG_INFO("Send answer:\n{}", message.dump().c_str());

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
  }
  return 0;
}

int IceTransport::SendOfferLocalCandidate(const std::string &remote_candidate) {
  json message = {{"type", "offer_candidate"},
                  {"transport_id", transport_id_},
                  {"sdp", remote_candidate}};
  LOG_INFO("Send candidate:\n{}", message.dump().c_str());

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
  }
  return 0;
}

int IceTransport::SendAnswerLocalCandidate(
    const std::string &remote_candidate) {
  json message = {{"type", "answer_candidate"},
                  {"transport_id", transport_id_},
                  {"sdp", remote_candidate}};
  LOG_INFO("Send candidate:\n{}", message.dump().c_str());

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
  }
  return 0;
}

int IceTransport::SendData(const char *data, size_t size) {
  ice_agent_->Send(data, size);
  return 0;
}

void IceTransport::OnReceiveMessage(const std::string &msg) {
  auto j = json::parse(msg);
  LOG_INFO("msg: {}", msg.c_str());

  std::string type = j["type"];
  auto itr = siganl_types.find(type);
  if (itr != siganl_types.end()) {
    LOG_INFO("msg type :{}", itr->first);
    switch (itr->second) {
      case 2: {
        break;
      }
      case 3: {
        transport_id_ = j["transport_id"].get<std::string>();
        LOG_INFO("Receive local peer transport_id [{}]", transport_id_);
        // SendOffer();
        break;
      }
      case 4: {
        remote_sdp_ = j["sdp"].get<std::string>();
        LOG_INFO("Receive remote sdp [{}]", remote_sdp_);
        SetRemoteSdp(remote_sdp_);

        if (!offer_peer_) {
          GatherCandidates();
        }
        break;
      }
      case 5: {
        std::string candidate = j["sdp"].get<std::string>();
        LOG_INFO("Receive candidate [{}]", candidate);
        AddRemoteCandidate(candidate);
        break;
      }
      default:
        break;
    }
  }
}
