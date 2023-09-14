#include "rtp_statistics.h"

#include "log.h"

RtpStatistics::RtpStatistics() {}

RtpStatistics::~RtpStatistics() {}

void RtpStatistics::UpdateSentBytes(uint32_t sent_bytes) {
  sent_bytes_ += sent_bytes;
}

void RtpStatistics::UpdateReceiveBytes(uint32_t received_bytes) {
  received_bytes_ += received_bytes;
}

bool RtpStatistics::Process() {
  if (!sent_bytes_) {
    // LOG_INFO("rtp statistics: Send [{} bps]", sent_bytes_);
  }

  if (!received_bytes_) {
    // LOG_INFO("rtp statistics: Receive [{} bps]", received_bytes_);
  }

  sent_bytes_ = 0;
  received_bytes_ = 0;

  std::this_thread::sleep_for(std::chrono::seconds(1));
  return true;
}