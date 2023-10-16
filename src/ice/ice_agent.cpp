#include "ice_agent.h"

#include <string.h>

#include <iostream>

#include "log.h"

IceAgent::IceAgent(bool offer_peer, std::string &stun_ip, uint16_t stun_port,
                   std::string &turn_ip, uint16_t turn_port,
                   std::string &turn_username, std::string &turn_password)
    : stun_ip_(stun_ip),
      stun_port_(stun_port),
      turn_ip_(turn_ip),
      turn_port_(turn_port),
      turn_username_(turn_username),
      turn_password_(turn_password),
      controlling_(offer_peer) {}

IceAgent::~IceAgent() {
  exit_thread_ = TRUE;
  g_thread_join(gexamplethread_);
}

void *IceAgent::CreateNiceAgent(void *data) {
  if (!data) {
    return nullptr;
  }

  IceAgent *ice_agent_ptr = (IceAgent *)data;

  ice_agent_ptr->gloop_ = g_main_loop_new(NULL, FALSE);

  // Create the nice agent_
  ice_agent_ptr->agent_ =
      nice_agent_new_reliable(g_main_loop_get_context(ice_agent_ptr->gloop_),
                              NICE_COMPATIBILITY_RFC5245);
  if (ice_agent_ptr->agent_ == NULL) {
    LOG_ERROR("Failed to create agent_");
  }

  g_object_set(ice_agent_ptr->agent_, "stun-server",
               ice_agent_ptr->stun_ip_.c_str(), NULL);
  g_object_set(ice_agent_ptr->agent_, "stun-server-port",
               ice_agent_ptr->stun_port_, NULL);

  g_object_set(ice_agent_ptr->agent_, "controlling-mode",
               ice_agent_ptr->controlling_, NULL);

  // Connect to the signals
  g_signal_connect(ice_agent_ptr->agent_, "candidate-gathering-done",
                   G_CALLBACK(ice_agent_ptr->on_gathering_done_),
                   ice_agent_ptr->user_ptr_);
  g_signal_connect(ice_agent_ptr->agent_, "new-selected-pair",
                   G_CALLBACK(ice_agent_ptr->on_candidate_),
                   ice_agent_ptr->user_ptr_);
  g_signal_connect(ice_agent_ptr->agent_, "component-state-changed",
                   G_CALLBACK(ice_agent_ptr->on_state_changed_),
                   ice_agent_ptr->user_ptr_);

  // Create a new stream with one component
  ice_agent_ptr->stream_id_ = nice_agent_add_stream(ice_agent_ptr->agent_, 1);
  if (ice_agent_ptr->stream_id_ == 0) {
    LOG_ERROR("Failed to add stream");
  }

  // nice_agent_set_stream_name(ice_agent_ptr->agent_, stream_id_, "video");

  // Attach to the component to receive the data
  // Without this call, candidates cannot be gathered
  nice_agent_attach_recv(ice_agent_ptr->agent_, ice_agent_ptr->stream_id_, 1,
                         g_main_loop_get_context(ice_agent_ptr->gloop_),
                         ice_agent_ptr->on_recv_, ice_agent_ptr->user_ptr_);

  g_main_loop_run(ice_agent_ptr->gloop_);
}

int IceAgent::CreateIceAgent(nice_cb_state_changed_t on_state_changed,
                             nice_cb_candidate_t on_candidate,
                             nice_cb_gathering_done_t on_gathering_done,
                             nice_cb_recv_t on_recv, void *user_ptr) {
  on_state_changed_ = on_state_changed;
  on_candidate_ = on_candidate;
  on_gathering_done_ = on_gathering_done;
  on_recv_ = on_recv;
  user_ptr_ = user_ptr;

  g_networking_init();

  // gloop_ = g_main_loop_new(NULL, FALSE);
  exit_thread_ = FALSE;
  // gexamplethread_ = g_thread_new("example thread", &CreateNiceAgent, this);

  // g_main_loop_run(gloop_);
  g_thread_.reset(new std::thread(std::bind(&IceAgent::CreateNiceAgent, this)));

  LOG_INFO("Nice agent init finish");
  g_usleep(100000);

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