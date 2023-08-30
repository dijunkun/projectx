#include "ice_transmission.h"

#include <chrono>
#include <map>
#include <nlohmann/json.hpp>
#include <thread>

#include "common.h"
#include "log.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <windows.h>
#elif !defined(__unix)
#define __unix
#endif

#ifdef __unix
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

using nlohmann::json;
static int count = 1;

static inline void itimeofday(long *sec, long *usec) {
#if defined(__unix)
  struct timeval time;
  gettimeofday(&time, NULL);
  if (sec) *sec = time.tv_sec;
  if (usec) *usec = time.tv_usec;
#else
  static long mode = 0, addsec = 0;
  BOOL retval;
  static IINT64 freq = 1;
  IINT64 qpc;
  if (mode == 0) {
    retval = QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
    freq = (freq == 0) ? 1 : freq;
    retval = QueryPerformanceCounter((LARGE_INTEGER *)&qpc);
    addsec = (long)time(NULL);
    addsec = addsec - (long)((qpc / freq) & 0x7fffffff);
    mode = 1;
  }
  retval = QueryPerformanceCounter((LARGE_INTEGER *)&qpc);
  retval = retval * 2;
  if (sec) *sec = (long)(qpc / freq) + addsec;
  if (usec) *usec = (long)((qpc % freq) * 1000000 / freq);
#endif
}

static inline IINT64 iclock64(void) {
  long s, u;
  IINT64 value;
  itimeofday(&s, &u);
  value = ((IINT64)s) * 1000 + (u / 1000);
  return value;
}

static inline IUINT32 iclock() { return (IUINT32)(iclock64() & 0xfffffffful); }

const std::vector<std::string> ice_status = {
    "JUICE_STATE_DISCONNECTED", "JUICE_STATE_GATHERING",
    "JUICE_STATE_CONNECTING",   "JUICE_STATE_CONNECTED",
    "JUICE_STATE_COMPLETED",    "JUICE_STATE_FAILED"};

IceTransmission::IceTransmission(
    bool offer_peer, std::string &transmission_id, std::string &user_id,
    std::string &remote_user_id, WsTransmission *ice_ws_transmission,
    std::function<void(const char *, size_t, const char *, size_t)>
        on_receive_ice_msg)
    : offer_peer_(offer_peer),
      transmission_id_(transmission_id),
      user_id_(user_id),
      remote_user_id_(remote_user_id),
      ice_ws_transport_(ice_ws_transmission),
      on_receive_ice_msg_cb_(on_receive_ice_msg) {}

IceTransmission::~IceTransmission() {
  if (ice_agent_) {
    delete ice_agent_;
    ice_agent_ = nullptr;
  }
  ikcp_release(kcp_);
}

int IceTransmission::InitIceTransmission(std::string &ip, int port) {
  kcp_ = ikcp_create(0x11223344, (void *)this);
  ikcp_setoutput(kcp_,
                 [](const char *buf, int len, ikcpcb *kcp, void *user) -> int {
                   IceTransmission *ice_transmission_obj =
                       static_cast<IceTransmission *>(user);
                   LOG_ERROR("Real send size: {}", len);
                   return ice_transmission_obj->ice_agent_->Send(buf, len);
                 });
  // ikcp_wndsize(kcp_, 1280, 1280);
  ikcp_nodelay(kcp_, 0, 40, 0, 0);
  ikcp_setmtu(kcp_, 4000);
  // kcp_->rx_minrto = 10;
  // kcp_->fastresend = 1;
  std::thread kcp_update_thread([this]() {
    while (1) {
      auto clock = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count();
      mtx_.lock();
      ikcp_update(kcp_, iclock());

      int len = 0;
      int total_len = 0;
      while (1) {
        len = ikcp_recv(kcp_, kcp_complete_buffer_ + len, 1400);
        total_len += len;
        if (len <= 0) break;
      }

      mtx_.unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
  });
  kcp_update_thread.detach();

  ice_agent_ = new IceAgent(ip, port);

  ice_agent_->CreateIceAgent(
      [](juice_agent_t *agent, juice_state_t state, void *user_ptr) {
        if (user_ptr) {
          IceTransmission *ice_transmission_obj =
              static_cast<IceTransmission *>(user_ptr);
          LOG_INFO("[{}->{}] state_change: {}", ice_transmission_obj->user_id_,
                   ice_transmission_obj->remote_user_id_, ice_status[state]);
          ice_transmission_obj->state_ = state;
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
          if (ice_transmission_obj->on_receive_ice_msg_cb_) {
            LOG_ERROR("[{}] Receive size: {}", (void *)user_ptr, size);
            ice_transmission_obj->mtx_.lock();
            int ret = ikcp_input(ice_transmission_obj->kcp_, data, size);
            // ikcp_update(ice_transmission_obj->kcp_, iclock());
            LOG_ERROR("ikcp_input {}", ret);
            // auto clock =
            //     std::chrono::duration_cast<std::chrono::milliseconds>(
            //         std::chrono::system_clock::now().time_since_epoch())
            //         .count();

            // ikcp_update(ice_transmission_obj->kcp_, clock);

            ice_transmission_obj->mtx_.unlock();

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
    LOG_ERROR("[{}] Wanna send size: {}", (void *)this, size);
    mtx_.lock();

    if (ikcp_waitsnd(kcp_) > kcp_->snd_wnd) {
      // LOG_ERROR("Skip frame");
      // mtx_.unlock();
      // return 0;
      ikcp_flush(kcp_);
    }
    int ret = ikcp_send(kcp_, data, size / 100);
    LOG_ERROR("ikcp_send {}, wnd [{} | {}]", ret, ikcp_waitsnd(kcp_),
              kcp_->snd_wnd);
    mtx_.unlock();
    // ice_agent_->Send(data, size);
  }
  return 0;
}