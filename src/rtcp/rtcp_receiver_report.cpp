#include "rtcp_receiver_report.h"

RtcpReceiverReport::RtcpReceiverReport() {
  buffer_ = new uint8_t[DEFAULT_RR_SIZE];
  size_ = DEFAULT_RR_SIZE;
}

RtcpReceiverReport::~RtcpReceiverReport() {
  if (buffer_) {
    delete buffer_;
    buffer_ = nullptr;
  }

  size_ = 0;
}

void RtcpReceiverReport::SetReportBlock(RtcpReportBlock &rtcp_report_block) {
  reports_.push_back(rtcp_report_block);
}

void RtcpReceiverReport::SetReportBlock(
    std::vector<RtcpReportBlock> &rtcp_report_blocks) {
  reports_ = rtcp_report_blocks;
}

const uint8_t *RtcpReceiverReport::Encode() {
  int pos = rtcp_header_.Encode(DEFAULT_RTCP_VERSION, 0, DEFAULT_RR_BLOCK_NUM,
                                RTCP_TYPE::RR, DEFAULT_RR_SIZE, buffer_);

  return buffer_;
}