#include "rtp_video_sender.h"

#include <chrono>

#include "log.h"

RtpVideoSender::RtpVideoSender() {}

RtpVideoSender::~RtpVideoSender() { rtp_video_send_statistics_->Stop(); }

void RtpVideoSender::Enqueue(std::vector<RtpPacket>& rtp_packets) {
  if (!rtp_video_send_statistics_) {
    rtp_video_send_statistics_ = std::make_unique<RtpVideoSendStatistics>();
    rtp_video_send_statistics_->Start();
  }

  for (auto& rtp_packet : rtp_packets) {
    rtp_packe_queue_.push(rtp_packet);
  }
}

bool RtpVideoSender::Process() {
  last_send_bytes_ = 0;

  for (size_t i = 0; i < 10; i++)
    if (!rtp_packe_queue_.isEmpty()) {
      RtpPacket rtp_packet;
      rtp_packe_queue_.pop(rtp_packet);
      if (rtp_packet_send_func_) {
        rtp_packet_send_func_(rtp_packet);
        last_send_bytes_ += rtp_packet.Size();
      }
    }

  if (rtp_video_send_statistics_) {
    rtp_video_send_statistics_->UpdateSentBytes(last_send_bytes_);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  return true;
}