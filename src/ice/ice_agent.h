#ifndef _ICE_AGENT_H_
#define _ICE_AGENT_H_

#include "juice/juice.h"

class IceAgent {
 public:
  IceAgent();
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
  juice_agent_t* agent_ = nullptr;
  char local_sdp_[JUICE_MAX_SDP_STRING_LEN];
  juice_state_t state_;
};

#endif