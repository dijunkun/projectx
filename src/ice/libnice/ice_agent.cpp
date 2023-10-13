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

int IceAgent::CreateIceAgent(nice_cb_state_changed_t on_state_changed,
                             nice_cb_candidate_t on_candidate,
                             nice_cb_gathering_done_t on_gathering_done,
                             nice_cb_recv_t on_recv, void *user_ptr) {
  g_networking_init();

  gloop_ = g_main_loop_new(NULL, FALSE);
  // Create the nice agent_
  agent_ = nice_agent_new(g_main_loop_get_context(gloop_),
                          NICE_COMPATIBILITY_RFC5245);
  if (agent_ == NULL) {
    LOG_ERROR("Failed to create agent_");
  }

  g_object_set(agent_, "stun-server", stun_ip_.c_str(), NULL);
  g_object_set(agent_, "stun-server-port", stun_port_, NULL);

  g_object_set(agent_, "controlling-mode", controlling_, NULL);

  // Connect to the signals
  g_signal_connect(agent_, "candidate-gathering-done",
                   G_CALLBACK(on_gathering_done), NULL);
  g_signal_connect(agent_, "new-selected-pair", G_CALLBACK(on_candidate), NULL);
  g_signal_connect(agent_, "component-state-changed",
                   G_CALLBACK(on_state_changed), NULL);

  // Create a new stream with one component
  stream_id_ = nice_agent_add_stream(agent_, 1);
  if (stream_id_ == 0) {
    LOG_ERROR("Failed to add stream");
  }
  nice_agent_set_stream_name(agent_, stream_id_, "video");

  // Attach to the component to receive the data
  // Without this call, candidates cannot be gathered
  nice_agent_attach_recv(agent_, stream_id_, 1, g_main_loop_get_context(gloop_),
                         on_recv, NULL);

  LOG_INFO("Nice agent init finish");

  return 0;
}

int IceAgent::DestoryIceAgent() {
  g_object_unref(agent_);
  return 0;
}

char *IceAgent::GenerateLocalSdp() {
  if (nullptr == agent_) {
    LOG_INFO("agent_ is nullptr");
    return nullptr;
  }

  local_sdp_ = nice_agent_generate_local_sdp(agent_);
  // LOG_INFO("Generate local sdp:[\n{}]", local_sdp_);

  return local_sdp_;
}

int IceAgent::SetRemoteSdp(const char *remote_sdp) {
  int ret = nice_agent_parse_remote_sdp(agent_, remote_sdp);
  if (ret > 0) {
    return 0;
  } else {
    LOG_ERROR("Failed to parse remote data");
    return -1;
  }
}

int IceAgent::GatherCandidates() {
  if (!nice_agent_gather_candidates(agent_, stream_id_)) {
    LOG_ERROR("Failed to start candidate gathering");
    return -1;
  }

  return 0;
}

NiceComponentState IceAgent::GetIceState() {
  state_ = nice_agent_get_component_state(agent_, stream_id_, 1);

  return state_;
}

int IceAgent::Send(const char *data, size_t size) {
  if (NiceComponentState::NICE_COMPONENT_STATE_READY !=
      nice_agent_get_component_state(agent_, stream_id_, 1)) {
    return -1;
  }

  nice_agent_send(agent_, stream_id_, 1, size, data);
  return 0;
}