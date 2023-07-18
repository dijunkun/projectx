#include "ice_agent.h"

#include <string.h>

#include <iostream>

#include "log.h"

IceAgent::IceAgent(std::string &ip, uint16_t port) : ip_(ip), port_(port) {}

IceAgent::~IceAgent() {}

int IceAgent::CreateIceAgent(juice_cb_state_changed_t on_state_changed,
                             juice_cb_candidate_t on_candidate,
                             juice_cb_gathering_done_t on_gathering_done,
                             juice_cb_recv_t on_recv, void *user_ptr) {
  // juice_set_log_level(JUICE_LOG_LEVEL_DEBUG);

  juice_config_t config;
  memset(&config, 0, sizeof(config));

  LOG_INFO("stun server ip[{}] port[{}]", ip_, port_);

  // STUN server example
  config.stun_server_host = ip_.c_str();
  config.stun_server_port = port_;

  config.cb_state_changed = on_state_changed;
  config.cb_candidate = on_candidate;
  config.cb_gathering_done = on_gathering_done;
  config.cb_recv = on_recv;
  config.user_ptr = user_ptr;

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
  LOG_INFO("Generate local sdp:[\n{}]", local_sdp_);

  return local_sdp_;
}

int IceAgent::SetRemoteSdp(const char *remote_sdp) {
  LOG_INFO("Set remote sdp");
  juice_set_remote_description(agent_, remote_sdp);
  LOG_INFO("Remote description:[\n{}]", remote_sdp);

  return 0;
}

int IceAgent::GatherCandidates() {
  LOG_INFO("Gather candidates");
  juice_gather_candidates(agent_);

  return 0;
}

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
  juice_add_remote_candidate(agent_, remote_candidates);

  return 0;
}

int IceAgent::SetRemoteGatheringDone() {
  juice_set_remote_gathering_done(agent_);

  return 0;
}

int IceAgent::Send(const char *data, size_t size) {
  juice_send(agent_, data, size);
  return 0;
}