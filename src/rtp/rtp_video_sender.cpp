#include "rtp_video_sender.h"

#include <chrono>

#include "log.h"

RtpVideoSender::RtpVideoSender() {}

RtpVideoSender::~RtpVideoSender() {}

void RtpVideoSender::Enqueue(std::vector<RtpPacket>& rtp_packets) {
  for (auto& rtp_packet : rtp_packets) {
    rtp_packe_queue_.push(rtp_packet);
  }
}

bool RtpVideoSender::Process() {
  for (size_t i = 0; i < 50; i++)
    if (!rtp_packe_queue_.isEmpty()) {
      RtpPacket rtp_packet;
      rtp_packe_queue_.pop(rtp_packet);
      if (rtp_packet_send_func_) {
        rtp_packet_send_func_(rtp_packet);
      }
    }

  return true;
}