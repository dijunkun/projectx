#ifndef _ICE_AGENT_H_
#define _ICE_AGENT_H_

#include <iostream>

#include "juice/juice.h"
#include "nice/agent.h"

class IceAgent {
 public:
  IceAgent(bool offer_peer, std::string& stun_ip, uint16_t stun_port,
           std::string& turn_ip, uint16_t turn_port, std::string& turn_username,
           std::string& turn_password);
  ~IceAgent();

  int CreateIceAgent(juice_cb_state_changed_t on_state_changed,
                     juice_cb_candidate_t on_candidate,
                     juice_cb_gathering_done_t on_gathering_done,
                     juice_cb_recv_t on_recv, void* user_ptr);

  int DestoryIceAgent();

  char* GenerateLocalSdp();

  int SetRemoteSdp(const char* remote_sdp);

  int GatherCandidates();

  juice_state_t GetIceState();

  bool GetSelectedCandidates();

  bool GetSelectedAddresses();

  int AddRemoteCandidates(const char* remote_candidates);

  int SetRemoteGatheringDone();

  int Send(const char* data, size_t size);

 private:
  std::string stun_ip_ = "";
  uint16_t stun_port_ = 0;
  std::string turn_ip_ = "";
  uint16_t turn_port_ = 0;
  std::string turn_username_ = "";
  std::string turn_password_ = "";
  juice_agent_t* agent_ = nullptr;
  char local_sdp_[JUICE_MAX_SDP_STRING_LEN];
  juice_state_t state_;
  juice_config_t config_;
  juice_turn_server_t turn_server_;
};

#endif