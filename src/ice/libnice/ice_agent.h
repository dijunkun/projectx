#ifndef _ICE_AGENT_H_
#define _ICE_AGENT_H_

#include <iostream>

#include "gio/gnetworking.h"
#include "nice/agent.h"

#define NICE_MAX_SDP_STRING_LEN 4096

typedef void (*nice_cb_state_changed_t)(NiceAgent* agent, guint stream_id,
                                        guint component_id,
                                        NiceComponentState state,
                                        gpointer data);
typedef void (*nice_cb_candidate_t)(NiceAgent* agent, guint stream_id,
                                    guint component_id, const char* sdp,
                                    gpointer data);
typedef void (*nice_cb_gathering_done_t)(NiceAgent* agent, guint stream_id,
                                         gpointer data);
typedef void (*nice_cb_recv_t)(NiceAgent* agent, guint stream_id,
                               guint component_id, guint size, gchar* buffer,
                               gpointer data);

class IceAgent {
 public:
  IceAgent(std::string& stun_ip, uint16_t stun_port, std::string& turn_ip,
           uint16_t turn_port, std::string& turn_username,
           std::string& turn_password);
  ~IceAgent();

  int CreateIceAgent(nice_cb_state_changed_t on_state_changed,
                     nice_cb_candidate_t on_candidate,
                     nice_cb_gathering_done_t on_gathering_done,
                     nice_cb_recv_t on_recv, void* user_ptr);

  int DestoryIceAgent();

  char* GenerateLocalSdp();

  int SetRemoteSdp(const char* remote_sdp);

  int GatherCandidates();

  NiceComponentState GetIceState();

  int SetRemoteGatheringDone();

  int Send(const char* data, size_t size);

 private:
  std::string stun_ip_ = "";
  uint16_t stun_port_ = 0;
  std::string turn_ip_ = "";
  uint16_t turn_port_ = 0;
  std::string turn_username_ = "";
  std::string turn_password_ = "";
  NiceAgent* agent_ = nullptr;
  GMainLoop* gloop_;
  bool controlling_ = false;
  uint32_t stream_id_ = 0;
  // char local_sdp_[NICE_MAX_SDP_STRING_LEN];
  char* local_sdp_ = nullptr;
  NiceComponentState state_;
};

#endif