#ifndef _RTCP_SENDER_REPORT_H_
#define _RTCP_SENDER_REPORT_H_

// SR
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |V=2|P|   RC    |   PT=SR=200   |            length             |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        SSRC of sender                         |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |              NTP timestamp, most significant word             | sender
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ info
// |             NTP timestamp, least significant word             |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         RTP timestamp                         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                     sender's packet count                     |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                      sender's octet count                     |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |                 SSRC_1 (SSRC of first source)                 | report
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ block
// | fraction lost |       cumulative number of packets lost       | 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           extended highest sequence number received           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                      interarrival jitter                      |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         last SR (LSR)                         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                   delay since last SR (DLSR)                  |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |                 SSRC_2 (SSRC of second source)                | report
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ block
// :                               ...                             : 2
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#include <vector>

#include "rtcp_header.h"
#include "rtcp_packet.h"
#include "rtcp_report_block.h"

#define DEFAULT_SR_BLOCK_NUM 1
#define DEFAULT_SR_SIZE 52

class RtcpSenderReport {
 public:
  RtcpSenderReport();
  ~RtcpSenderReport();

 public:
  typedef struct {
    uint32_t sender_ssrc : 32;
    uint64_t ntp_ts : 64;
    uint32_t rtp_ts : 32;
    uint32_t sender_packet_count : 32;
    uint32_t sender_octet_count : 32;
  } SENDER_INFO;

  void SetSenderInfo(SENDER_INFO &sender_info) {
    sender_info_.sender_ssrc = sender_info.sender_ssrc;
    sender_info_.ntp_ts = sender_info.ntp_ts;
    sender_info_.rtp_ts = sender_info.rtp_ts;
    sender_info_.sender_packet_count = sender_info.sender_packet_count;
    sender_info_.sender_octet_count = sender_info.sender_octet_count;
  }

 public:
  const uint8_t *Encode();
  size_t Decode();

  // Entire RTP buffer
  const uint8_t *Buffer() const { return buffer_; }
  size_t Size() const { return size_; }

 private:
  RtcpHeader rtcp_header_;
  SENDER_INFO sender_info_;
  std::vector<RtcpReportBlock> reports_;

  // Entire RTCP buffer
  uint8_t *buffer_ = nullptr;
  size_t size_ = 0;
};

#endif