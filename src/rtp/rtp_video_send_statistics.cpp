#include "rtp_video_send_statistics.h"

#include "log.h"

RtpVideoSendStatistics::RtpVideoSendStatistics() {}

RtpVideoSendStatistics::~RtpVideoSendStatistics() {}

void RtpVideoSendStatistics::UpdateSentBytes(uint32_t sent_bytes) {
  sent_bytes_ += sent_bytes;
}

bool RtpVideoSendStatistics::Process() {
  LOG_INFO("rtp statistics: Send [{} bps]", sent_bytes_);
  sent_bytes_ = 0;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return true;
}