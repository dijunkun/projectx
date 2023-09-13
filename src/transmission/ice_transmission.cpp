#include "ice_transmission.h"

#include <chrono>
#include <map>
#include <nlohmann/json.hpp>
#include <thread>

#include "common.h"
#include "ikcp.h"
#include "log.h"

using nlohmann::json;

const std::vector<std::string> ice_status = {
    "JUICE_STATE_DISCONNECTED", "JUICE_STATE_GATHERING",
    "JUICE_STATE_CONNECTING",   "JUICE_STATE_CONNECTED",
    "JUICE_STATE_COMPLETED",    "JUICE_STATE_FAILED"};

IceTransmission::IceTransmission(
    bool offer_peer, std::string &transmission_id, std::string &user_id,
    std::string &remote_user_id,
    std::shared_ptr<WsTransmission> ice_ws_transmission,
    std::function<void(const char *, size_t, const char *, size_t)>
        on_receive_ice_msg,
    std::function<void(std::string)> on_ice_status_change)
    : offer_peer_(offer_peer),
      transmission_id_(transmission_id),
      user_id_(user_id),
      remote_user_id_(remote_user_id),
      ice_ws_transport_(ice_ws_transmission),
      on_receive_ice_msg_cb_(on_receive_ice_msg),
      on_ice_status_change_(on_ice_status_change) {}

IceTransmission::~IceTransmission() {
  if (rtp_video_sender_) {
    rtp_video_sender_->Stop();
  }

  if (rtp_video_receiver_) {
    rtp_video_receiver_->Stop();
  }

  if (rtp_data_sender_) {
    rtp_data_sender_->Stop();
  }

  if (rtp_payload_) {
    delete rtp_payload_;
    rtp_payload_ = nullptr;
  }
}

int IceTransmission::InitIceTransmission(std::string &ip, int port) {
  video_rtp_codec_ = std::make_unique<RtpCodec>(RtpPacket::PAYLOAD_TYPE::H264);
  data_rtp_codec_ = std::make_unique<RtpCodec>(RtpPacket::PAYLOAD_TYPE::DATA);

  rtp_video_receiver_ = std::make_unique<RtpVideoReceiver>();
  rtp_video_receiver_->SetSendDataFunc(
      [this](const char *data, size_t size) -> int {
        if (!ice_agent_) {
          LOG_ERROR("ice_agent_ is nullptr");
          return -1;
        }

        return ice_agent_->Send(data, size);
      });
  rtp_video_receiver_->SetOnReceiveCompleteFrame(
      [this](VideoFrame &video_frame) -> void {
        // LOG_ERROR("OnReceiveCompleteFrame {}", video_frame.Size());
        on_receive_ice_msg_cb_((const char *)video_frame.Buffer(),
                               video_frame.Size(), remote_user_id_.data(),
                               remote_user_id_.size());
      });

  rtp_video_receiver_->Start();

  rtp_video_sender_ = std::make_unique<RtpVideoSender>();
  rtp_video_sender_->SetSendDataFunc(
      [this](const char *data, size_t size) -> int {
        if (!ice_agent_) {
          LOG_ERROR("ice_agent_ is nullptr");
          return -1;
        }

        return ice_agent_->Send(data, size);
      });

  rtp_video_sender_->Start();

  rtp_data_sender_ = std::make_unique<RtpDataSender>();
  rtp_data_sender_->SetSendDataFunc(
      [this](const char *data, size_t size) -> int {
        if (!ice_agent_) {
          LOG_ERROR("ice_agent_ is nullptr");
          return -1;
        }

        return ice_agent_->Send(data, size);
      });

  rtp_data_sender_->Start();

  ice_agent_ = std::make_unique<IceAgent>(ip, port);

  ice_agent_->CreateIceAgent(
      [](juice_agent_t *agent, juice_state_t state, void *user_ptr) {
        if (user_ptr) {
          IceTransmission *ice_transmission_obj =
              static_cast<IceTransmission *>(user_ptr);
          LOG_INFO("[{}->{}] state_change: {}", ice_transmission_obj->user_id_,
                   ice_transmission_obj->remote_user_id_, ice_status[state]);
          ice_transmission_obj->state_ = state;
          ice_transmission_obj->on_ice_status_change_(ice_status[state]);
        } else {
          LOG_INFO("state_change: {}", ice_status[state]);
        }
      },
      [](juice_agent_t *agent, const char *sdp, void *user_ptr) {
        // LOG_INFO("candadite: {}", sdp);
        // trickle
        // static_cast<IceTransmission
        // *>(user_ptr)->SendOfferLocalCandidate(sdp);
      },
      [](juice_agent_t *agent, void *user_ptr) {
        // non-trickle
        if (user_ptr) {
          IceTransmission *ice_transmission_obj =
              static_cast<IceTransmission *>(user_ptr);
          LOG_INFO("[{}] gather_done", ice_transmission_obj->user_id_);

          if (ice_transmission_obj->offer_peer_) {
            ice_transmission_obj->GetLocalSdp();
            ice_transmission_obj->SendOffer();

          } else {
            ice_transmission_obj->CreateAnswer();
            ice_transmission_obj->SendAnswer();
          }
        }
      },
      [](juice_agent_t *agent, const char *buffer, size_t size,
         void *user_ptr) {
        if (user_ptr) {
          IceTransmission *ice_transmission_obj =
              static_cast<IceTransmission *>(user_ptr);
          if (ice_transmission_obj) {
            if (ice_transmission_obj->CheckIsVideoPacket(buffer, size)) {
              RtpPacket packet((uint8_t *)buffer, size);
              ice_transmission_obj->rtp_video_receiver_->InsertRtpPacket(
                  packet);
            } else if (ice_transmission_obj->CheckIsRtcpPacket(buffer, size)) {
              LOG_ERROR("Rtcp packet [{}]", (uint8_t)(buffer[1]));
            }
          }
        }
      },
      this);
  return 0;
}

int IceTransmission::DestroyIceTransmission() {
  LOG_INFO("[{}->{}] Destroy ice transmission", user_id_, remote_user_id_);
  return ice_agent_->DestoryIceAgent();
}

int IceTransmission::SetTransmissionId(const std::string &transmission_id) {
  transmission_id_ = transmission_id;

  return 0;
}

int IceTransmission::JoinTransmission() {
  LOG_INFO("[{}] Join transmission", user_id_);

  CreateOffer();
  return 0;
}

int IceTransmission::GatherCandidates() {
  ice_agent_->GatherCandidates();
  LOG_INFO("[{}] Gather candidates", user_id_);
  return 0;
}

int IceTransmission::GetLocalSdp() {
  local_sdp_ = ice_agent_->GenerateLocalSdp();
  LOG_INFO("[{}] generate local sdp", user_id_);
  return 0;
}

int IceTransmission::SetRemoteSdp(const std::string &remote_sdp) {
  ice_agent_->SetRemoteSdp(remote_sdp.c_str());
  LOG_INFO("[{}] set remote sdp", user_id_);
  remote_ice_username_ = GetIceUsername(remote_sdp);
  return 0;
}

int IceTransmission::AddRemoteCandidate(const std::string &remote_candidate) {
  ice_agent_->AddRemoteCandidates(remote_candidate.c_str());
  return 0;
}

int IceTransmission::CreateOffer() {
  LOG_INFO("[{}] create offer", user_id_);
  GatherCandidates();
  return 0;
}

int IceTransmission::SendOffer() {
  json message = {{"type", "offer"},
                  {"transmission_id", transmission_id_},
                  {"user_id", user_id_},
                  {"remote_user_id", remote_user_id_},
                  {"sdp", local_sdp_}};
  // LOG_INFO("Send offer:\n{}", message.dump());

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
    LOG_INFO("[{}->{}] send offer", user_id_, remote_user_id_);
  }
  return 0;
}

int IceTransmission::CreateAnswer() {
  GetLocalSdp();
  return 0;
}

int IceTransmission::SendAnswer() {
  json message = {{"type", "answer"},
                  {"transmission_id", transmission_id_},
                  {"sdp", local_sdp_},
                  {"user_id", user_id_},
                  {"remote_user_id", remote_user_id_}};

  if (ice_ws_transport_) {
    ice_ws_transport_->Send(message.dump());
    LOG_INFO("[{}->{}] send answer", user_id_, remote_user_id_);
  }
  return 0;
}

int IceTransmission::SendData(DATA_TYPE type, const char *data, size_t size) {
  if (JUICE_STATE_COMPLETED == state_) {
    std::vector<RtpPacket> packets;

    if (DATA_TYPE::VIDEO == type) {
      if (rtp_video_sender_) {
        if (video_rtp_codec_) {
          video_rtp_codec_->Encode((uint8_t *)data, size, packets);
        }
        rtp_video_sender_->Enqueue(packets);
      }
    } else if (DATA_TYPE::AUDIO == type) {
    } else if (DATA_TYPE::DATA == type) {
      if (rtp_data_sender_) {
        if (data_rtp_codec_) {
          data_rtp_codec_->Encode((uint8_t *)data, size, packets);
        }
        rtp_data_sender_->Enqueue(packets);
      }
    }
  }
  return 0;
}

uint8_t IceTransmission::CheckIsRtcpPacket(const char *buffer, size_t size) {
  if (size < 4) {
    return 0;
  }

  uint8_t pt = buffer[1];

  switch (pt) {
    case RtcpHeader::PAYLOAD_TYPE::SR: {
      return pt;
    }
    case RtcpHeader::PAYLOAD_TYPE::RR: {
      return pt;
    }
    case RtcpHeader::PAYLOAD_TYPE::SDES: {
      return pt;
    }
    case RtcpHeader::PAYLOAD_TYPE::BYE: {
      return pt;
    }
    case RtcpHeader::PAYLOAD_TYPE::APP: {
      return pt;
    }
    default: {
      return 0;
    }
  }
}

uint8_t IceTransmission::CheckIsVideoPacket(const char *buffer, size_t size) {
  if (size < 4) {
    return 0;
  }

  uint8_t pt = buffer[1] & 0x7F;
  if (RtpPacket::PAYLOAD_TYPE::H264 == pt) {
    return pt;
  } else {
    return 0;
  }
}

uint8_t IceTransmission::CheckIsAudioPacket(const char *buffer, size_t size) {
  if (size < 4) {
    return 0;
  }

  uint8_t pt = buffer[1] & 0x7F;
  if (RtpPacket::PAYLOAD_TYPE::OPUS == pt) {
    return pt;
  } else {
    return 0;
  }
}