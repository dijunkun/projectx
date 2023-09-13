#include "rtcp_sender_report.h"

RtcpSenderReport::RtcpSenderReport() {
  buffer_ = new uint8_t[DEFAULT_SR_SIZE];
  size_ = DEFAULT_SR_SIZE;
}

RtcpSenderReport::~RtcpSenderReport() {
  if (buffer_) {
    delete buffer_;
    buffer_ = nullptr;
  }

  size_ = 0;
}

void RtcpSenderReport::SetSenderInfo(SenderInfo &sender_info) {
  sender_info_.sender_ssrc = sender_info.sender_ssrc;
  sender_info_.ntp_ts_msw = sender_info.ntp_ts_msw;
  sender_info_.ntp_ts_lsw = sender_info.ntp_ts_lsw;
  sender_info_.rtp_ts = sender_info.rtp_ts;
  sender_info_.sender_packet_count = sender_info.sender_packet_count;
  sender_info_.sender_octet_count = sender_info.sender_octet_count;
}

void RtcpSenderReport::SetReportBlock(RtcpReportBlock &rtcp_report_block) {
  reports_.push_back(rtcp_report_block);
}

void RtcpSenderReport::SetReportBlock(
    std::vector<RtcpReportBlock> &rtcp_report_blocks) {
  reports_ = rtcp_report_blocks;
}

const uint8_t *RtcpSenderReport::Encode() {
  int pos = rtcp_header_.Encode(DEFAULT_RTCP_VERSION, 0, DEFAULT_SR_BLOCK_NUM,
                                RTCP_TYPE::SR, DEFAULT_SR_SIZE, buffer_);

  buffer_[pos] = sender_info_.sender_ssrc >> 24 & 0xFF;
  buffer_[pos + 1] = sender_info_.sender_ssrc >> 16 & 0xFF;
  buffer_[pos + 2] = sender_info_.sender_ssrc >> 8 & 0xFF;
  buffer_[pos + 3] = sender_info_.sender_ssrc & 0xFF;

  buffer_[pos + 4] = sender_info_.ntp_ts_msw >> 56 & 0xFF;
  buffer_[pos + 5] = sender_info_.ntp_ts_msw >> 48 & 0xFF;
  buffer_[pos + 6] = sender_info_.ntp_ts_msw >> 40 & 0xFF;
  buffer_[pos + 7] = sender_info_.ntp_ts_msw >> 32 & 0xFF;
  buffer_[pos + 8] = sender_info_.ntp_ts_lsw >> 24 & 0xFF;
  buffer_[pos + 9] = sender_info_.ntp_ts_lsw >> 16 & 0xFF;
  buffer_[pos + 10] = sender_info_.ntp_ts_lsw >> 8 & 0xFF;
  buffer_[pos + 11] = sender_info_.ntp_ts_lsw & 0xFF;

  buffer_[pos + 12] = sender_info_.rtp_ts >> 24 & 0xFF;
  buffer_[pos + 13] = sender_info_.rtp_ts >> 16 & 0xFF;
  buffer_[pos + 14] = sender_info_.rtp_ts >> 8 & 0xFF;
  buffer_[pos + 15] = sender_info_.rtp_ts & 0xFF;

  buffer_[pos + 16] = sender_info_.sender_packet_count >> 24 & 0xFF;
  buffer_[pos + 17] = sender_info_.sender_packet_count >> 16 & 0xFF;
  buffer_[pos + 18] = sender_info_.sender_packet_count >> 8 & 0xFF;
  buffer_[pos + 19] = sender_info_.sender_packet_count & 0xFF;

  buffer_[pos + 20] = sender_info_.sender_octet_count >> 24 & 0xFF;
  buffer_[pos + 21] = sender_info_.sender_octet_count >> 16 & 0xFF;
  buffer_[pos + 22] = sender_info_.sender_octet_count >> 8 & 0xFF;
  buffer_[pos + 23] = sender_info_.sender_octet_count & 0xFF;

  return buffer_;
}