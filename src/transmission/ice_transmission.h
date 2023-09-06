#ifndef _ICE_TRANSMISSION_H_
#define _ICE_TRANSMISSION_H_

#include <iostream>

#include "congestion_control.h"
#include "ice_agent.h"
#include "ringbuffer.h"
#include "rtp_packet.h"
#include "rtp_session.h"
#include "ws_transmission.h"
class IceTransmission {
 public:
  IceTransmission(
      bool offer_peer, std::string &transmission_id, std::string &user_id,
      std::string &remote_user_id, WsTransmission *ice_ws_transmission,
      std::function<void(const char *, size_t, const char *, size_t)>
          on_receive_ice_msg);

  ~IceTransmission();

 public:
  int InitIceTransmission(std::string &ip, int port);

  int DestroyIceTransmission();

  int JoinTransmission();

  int SetTransmissionId(const std::string &transmission_id);

  int SendData(const char *data, size_t size);

 public:
  int GatherCandidates();

  int GetLocalSdp();

  int SetRemoteSdp(const std::string &remote_sdp);

  int AddRemoteCandidate(const std::string &remote_candidate);

  int CreateOffer();

  int SendOffer();

  int CreateAnswer();

  int SendAnswer();

 private:
  IceAgent *ice_agent_ = nullptr;
  WsTransmission *ice_ws_transport_ = nullptr;
  CongestionControl *congestion_control_ = nullptr;
  std::function<void(const char *, size_t, const char *, size_t)>
      on_receive_ice_msg_cb_ = nullptr;
  std::string local_sdp_;
  std::string remote_sdp_;
  std::string local_candidates_;
  std::string remote_candidates_;
  unsigned int connection_id_ = 0;
  std::string transmission_id_ = "";
  std::string user_id_ = "";
  std::string remote_user_id_ = "";
  bool offer_peer_ = true;
  std::string remote_ice_username_ = "";
  juice_state_t state_ = JUICE_STATE_DISCONNECTED;

 private:
  // ikcpcb *kcp_ = nullptr;
  char kcp_complete_buffer_[2560 * 1440 * 4];
  std::mutex mtx_;
  RingBuffer<RtpPacket> send_ringbuffer_;
  RingBuffer<RtpPacket> recv_ringbuffer_;
  bool kcp_stop_ = false;
  std::thread *kcp_update_thread_ = nullptr;

 private:
  RtpSession *rtp_session_ = nullptr;
};

#endif