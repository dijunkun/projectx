#include "ice_agent.h"

#include <string.h>

#include <iostream>

#include "log.h"

IceAgent::IceAgent(std::string &stun_ip, uint16_t stun_port,
                   std::string &turn_ip, uint16_t turn_port,
                   std::string &turn_username, std::string &turn_password)
    : stun_ip_(stun_ip),
      stun_port_(stun_port),
      turn_ip_(turn_ip),
      turn_port_(turn_port),
      turn_username_(turn_username),
      turn_password_(turn_password) {}

IceAgent::~IceAgent() {}

int IceAgent::CreateIceAgent(juice_cb_state_changed_t on_state_changed,
                             juice_cb_candidate_t on_candidate,
                             juice_cb_gathering_done_t on_gathering_done,
                             juice_cb_recv_t on_recv, void *user_ptr) {
  juice_set_log_level(JUICE_LOG_LEVEL_DEBUG);

  juice_set_log_handler([](juice_log_level_t level, const char *message) {
    if (JUICE_LOG_LEVEL_VERBOSE == level) {
      LOG_INFO("{}", message);
    } else if (JUICE_LOG_LEVEL_DEBUG == level) {
      LOG_INFO("{}", message);
    } else if (JUICE_LOG_LEVEL_INFO == level) {
      LOG_INFO("{}", message);
    } else if (JUICE_LOG_LEVEL_WARN == level) {
      LOG_WARN("{}", message);
    } else if (JUICE_LOG_LEVEL_ERROR == level) {
      LOG_ERROR("{}", message);
    } else if (JUICE_LOG_LEVEL_FATAL == level) {
      LOG_FATAL("{}", message);
    } else if (JUICE_LOG_LEVEL_NONE == level) {
      LOG_INFO("{}", message);
    }
  });

  juice_config_t config;
  memset(&config, 0, sizeof(config));

  // STUN server example
  config.stun_server_host = stun_ip_.c_str();
  config.stun_server_port = stun_port_;

  if (!turn_ip_.empty() && -1 != turn_port_ && !turn_username_.empty() &&
      !turn_password_.empty()) {
    juice_turn_server_t turn_server;
    memset(&turn_server, 0, sizeof(turn_server));
    turn_server.host = turn_ip_.c_str();
    turn_server.port = turn_port_;
    turn_server.username = turn_username_.c_str();
    turn_server.password = turn_password_.c_str();
    config.turn_servers = &turn_server;
    config.turn_servers_count = 1;
  }

  config.cb_state_changed = on_state_changed;
  config.cb_candidate = on_candidate;
  config.cb_gathering_done = on_gathering_done;
  config.cb_recv = on_recv;
  config.user_ptr = user_ptr;

  // config.local_port_range_begin = 40000;
  // config.local_port_range_end = 50000;

  agent_ = juice_create(&config);

  return 0;
}

int IceAgent::DestoryIceAgent() {
  juice_destroy(agent_);
  return 0;
}

char *IceAgent::GenerateLocalSdp() {
  if (nullptr == agent_) {
    LOG_INFO("agent_ is nullptr");
    return nullptr;
  }

  juice_get_local_description(agent_, local_sdp_, JUICE_MAX_SDP_STRING_LEN);
  // LOG_INFO("Generate local sdp:[\n{}]", local_sdp_);

  return local_sdp_;
}

int IceAgent::SetRemoteSdp(const char *remote_sdp) {
  return juice_set_remote_description(agent_, remote_sdp);
}

int IceAgent::GatherCandidates() { return juice_gather_candidates(agent_); }

juice_state_t IceAgent::GetIceState() {
  state_ = juice_get_state(agent_);

  return state_;
}

bool IceAgent::GetSelectedCandidates() {
  char local[JUICE_MAX_CANDIDATE_SDP_STRING_LEN];
  char remote[JUICE_MAX_CANDIDATE_SDP_STRING_LEN];

  bool success = state_ == JUICE_STATE_COMPLETED;
  if (success &= (juice_get_selected_candidates(
                      agent_, local, JUICE_MAX_CANDIDATE_SDP_STRING_LEN, remote,
                      JUICE_MAX_CANDIDATE_SDP_STRING_LEN) == 0)) {
    LOG_INFO("Local candidate  1: {}", local);
    LOG_INFO("Remote candidate 1: {}", remote);
    if ((!strstr(local, "typ host") && !strstr(local, "typ prflx")) ||
        (!strstr(remote, "typ host") && !strstr(remote, "typ prflx")))
      success = false;  // local connection should be possible
  }

  return success;
}

bool IceAgent::GetSelectedAddresses() {
  char localAddr[JUICE_MAX_ADDRESS_STRING_LEN];
  char remoteAddr[JUICE_MAX_ADDRESS_STRING_LEN];

  bool success = state_ == JUICE_STATE_COMPLETED;
  if (success &= (juice_get_selected_addresses(
                      agent_, localAddr, JUICE_MAX_ADDRESS_STRING_LEN,
                      remoteAddr, JUICE_MAX_ADDRESS_STRING_LEN) == 0)) {
    LOG_INFO("Local address  1: {}", localAddr);
    LOG_INFO("Remote address 1: {}", remoteAddr);
  }

  return success;
}

int IceAgent::AddRemoteCandidates(const char *remote_candidates) {
  return juice_add_remote_candidate(agent_, remote_candidates);
}

int IceAgent::SetRemoteGatheringDone() {
  return juice_set_remote_gathering_done(agent_);
}

int IceAgent::Send(const char *data, size_t size) {
  return juice_send(agent_, data, size);
}