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
    std::string &remote_user_id, WsTransmission *ice_ws_transmission,
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
  if (kcp_update_thread_ && kcp_update_thread_->joinable()) {
    kcp_update_thread_->join();
    delete kcp_update_thread_;
    kcp_update_thread_ = nullptr;
  }

  if (video_rtp_session_) {
    delete video_rtp_session_;
    video_rtp_session_ = nullptr;
  }

  if (rtp_payload_) {
    delete rtp_payload_;
    rtp_payload_ = nullptr;
  }

  if (ice_agent_) {
    delete ice_agent_;
    ice_agent_ = nullptr;
  }
}

int IceTransmission::InitIceTransmission(std::string &ip, int port) {
  kcp_update_thread_ = new std::thread([this]() {
    int ret = 0;
    ikcpcb *kcp = ikcp_create(0x11223344, (void *)this);
    ikcp_setoutput(
        kcp, [](const char *buf, int len, ikcpcb *kcp, void *user) -> int {
          IceTransmission *ice_transmission_obj =
              static_cast<IceTransmission *>(user);
          return ice_transmission_obj->ice_agent_->Send(buf, len);
        });
    ikcp_wndsize(kcp, 2048, 2048);
    ikcp_nodelay(kcp, 1, 20, 2, 1);
    // ikcp_setmtu(kcp, 4000);
    // kcp_->rx_minrto = 10;
    // kcp_->fastresend = 1;

    while (!kcp_stop_) {
      auto clock = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count();

      ikcp_update(kcp, clock);

      if (!send_ringbuffer_.isEmpty()) {
        // Data buffer;
        RtpPacket buffer;
        if (ikcp_waitsnd(kcp) <= kcp->snd_wnd * 2) {
          send_ringbuffer_.pop(buffer);

          ice_agent_->Send((const char *)buffer.Buffer(), buffer.Size());
        }
      }

      if (!recv_ringbuffer_.isEmpty()) {
        // RtpPacket packet;
        recv_ringbuffer_.pop(pop_packet_);
        if (!rtp_payload_) {
          rtp_payload_ = new uint8_t[1400];
        }
        size_t rtp_payload_size =
            video_rtp_session_->Decode(pop_packet_, rtp_payload_);

        on_receive_ice_msg_cb_((const char *)rtp_payload_, rtp_payload_size,
                               remote_user_id_.data(), remote_user_id_.size());
      }

      // int len = 0;
      // int total_len = 0;
      // while (1) {
      //   len = ikcp_recv(kcp, kcp_complete_buffer_ + len, 400000);

      //   total_len += len;

      //   if (len <= 0) {
      //     if (on_receive_ice_msg_cb_ && total_len > 0) {
      //       LOG_ERROR("Receive size: {}", total_len);
      //       on_receive_ice_msg_cb_(kcp_complete_buffer_, total_len,
      //                              remote_user_id_.data(),
      //                              remote_user_id_.size());
      //     }
      //     break;
      //   }
      // }

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    ikcp_release(kcp);
  });

  video_rtp_session_ = new RtpSession(PAYLOAD_TYPE::H264);
  ice_agent_ = new IceAgent(ip, port);

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
      [](juice_agent_t *agent, const char *data, size_t size, void *user_ptr) {
        if (user_ptr) {
          IceTransmission *ice_transmission_obj =
              static_cast<IceTransmission *>(user_ptr);
          if (ice_transmission_obj) {
            // ice_transmission_obj->recv_ringbuffer_.push(
            //     std::move(Data(data, size)));

            RtpPacket packet((uint8_t *)data, size);
            ice_transmission_obj->recv_ringbuffer_.push(packet);

            // int ret = ikcp_input(ice_transmission_obj->kcp_, data, size);
            // ikcp_update(ice_transmission_obj->kcp_, iclock());
            // LOG_ERROR("ikcp_input {}", ret);
            // auto clock =
            //     std::chrono::duration_cast<std::chrono::milliseconds>(
            //         std::chrono::system_clock::now().time_since_epoch())
            //         .count();

            // ikcp_update(ice_transmission_obj->kcp_, clock);

            // ice_transmission_obj->on_receive_ice_msg_cb_(
            //     ice_transmission_obj->kcp_complete_buffer_, total_len,
            //     ice_transmission_obj->remote_user_id_.data(),
            //     ice_transmission_obj->remote_user_id_.size());

            // ice_transmission_obj->on_receive_ice_msg_cb_(
            //     data, size, ice_transmission_obj->remote_user_id_.data(),
            //     ice_transmission_obj->remote_user_id_.size());
          }
        }
      },
      this);
  return 0;
}

int IceTransmission::DestroyIceTransmission() {
  LOG_INFO("[{}->{}] Destroy ice transmission", user_id_, remote_user_id_);
  kcp_stop_ = true;
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

int IceTransmission::SendData(const char *data, size_t size) {
  if (JUICE_STATE_COMPLETED == state_) {
    // send_ringbuffer_.push(std::move(Data(data, size)));

    std::vector<RtpPacket> packets;

    for (size_t num = 0, next_packet_size = 0; num * MAX_NALU_LEN < size;
         num++) {
      next_packet_size = size - num * MAX_NALU_LEN;
      if (next_packet_size < MAX_NALU_LEN) {
        video_rtp_session_->Encode((uint8_t *)(data + num * MAX_NALU_LEN),
                                   next_packet_size, packets);
      } else {
        video_rtp_session_->Encode((uint8_t *)(data + num * MAX_NALU_LEN),
                                   MAX_NALU_LEN, packets);
      }
    }

    for (auto &packet : packets) {
      send_ringbuffer_.push(packet);
    }

    // std::vector<RtpPacket> packets =
    //     video_rtp_session_->Encode((uint8_t *)(data), size);

    // send_ringbuffer_.insert(send_ringbuffer_.end(), packets.begin(),
    //                         packets.end());
  }
  return 0;
}