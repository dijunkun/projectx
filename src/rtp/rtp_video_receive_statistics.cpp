#include "rtp_video_receive_statistics.h"

#include "log.h"

RtpVideoReceiveStatistics::RtpVideoReceiveStatistics() {}

RtpVideoReceiveStatistics::~RtpVideoReceiveStatistics() {}

void RtpVideoReceiveStatistics::UpdateReceiveBytes(uint32_t received_bytes) {
  received_bytes_ += received_bytes;
}

bool RtpVideoReceiveStatistics::Process() {
  LOG_INFO("rtp statistics: Receive [{} bps]", received_bytes_);
  received_bytes_ = 0;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return true;
}