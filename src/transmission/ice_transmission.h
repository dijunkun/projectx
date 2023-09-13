#ifndef _ICE_TRANSMISSION_H_
#define _ICE_TRANSMISSION_H_

#include <iostream>

#include "congestion_control.h"
#include "ice_agent.h"
#include "ringbuffer.h"
#include "rtp_codec.h"
#include "rtp_packet.h"
#include "rtp_video_receiver.h"
#include "rtp_video_sender.h"
#include "ws_transmission.h"

class IceTransmission {
 public:
  IceTransmission(
      bool offer_peer, std::string &transmission_id, std::string &user_id,
      std::string &remote_user_id,
      std::shared_ptr<WsTransmission> ice_ws_transmission,
      std::function<void(const char *, size_t, const char *, size_t)>
          on_receive_ice_msg,
      std::function<void(std::string)> on_ice_status_change);

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
  uint8_t CheckIsRtcpPacket(const char *buffer, size_t size);
  uint8_t CheckIsVideoPacket(const char *buffer, size_t size);
  uint8_t CheckIsAudioPacket(const char *buffer, size_t size);

 private:
  std::unique_ptr<IceAgent> ice_agent_ = nullptr;
  std::shared_ptr<WsTransmission> ice_ws_transport_ = nullptr;
  CongestionControl *congestion_control_ = nullptr;
  std::function<void(const char *, size_t, const char *, size_t)>
      on_receive_ice_msg_cb_ = nullptr;
  std::function<void(std::string)> on_ice_status_change_ = nullptr;
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
  std::unique_ptr<RtpCodec> rtp_codec_ = nullptr;
  std::unique_ptr<RtpVideoReceiver> rtp_video_receiver_ = nullptr;
  std::unique_ptr<RtpVideoSender> rtp_video_sender_ = nullptr;
  uint8_t *rtp_payload_ = nullptr;
  RtpPacket pop_packet_;
  bool start_send_packet_ = false;

  uint32_t last_complete_frame_ts_ = 0;
};

#endif