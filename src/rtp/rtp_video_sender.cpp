#include "rtp_video_sender.h"

#include <chrono>

RtpVideoSender::RtpVideoSender() {}

RtpVideoSender::~RtpVideoSender() {
  if (send_thread_ && send_thread_->joinable()) {
    send_thread_->join();
    delete send_thread_;
    send_thread_ = nullptr;
  }
}

void RtpVideoSender::Enqueue(std::vector<RtpPacket>& rtp_packets) {
  if (!send_thread_) {
    send_thread_ = new std::thread(&RtpVideoSender::Process, this);
  }

  for (auto& rtp_packet : rtp_packets) {
    start_ = true;
    rtp_packe_queue_.push(rtp_packet);
  }
}

void RtpVideoSender::Process() {
  while (1) {
    if (!rtp_packe_queue_.isEmpty()) {
      RtpPacket rtp_packet;
      rtp_packe_queue_.pop(rtp_packet);
      if (rtp_packet_send_func_) {
        rtp_packet_send_func_(rtp_packet);
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}