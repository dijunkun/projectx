#ifndef _RTCP_PACKET_H_
#define _RTCP_PACKET_H_

#include <stdint.h>

#include <vector>

// SR
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |V=2|P|   RC    |   PT=SR=200   |            length             | header
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
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |                  profile-specific extensions                  |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// RR
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |V=2|P|    RC   |   PT=RR=201   |             length            | header
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                     SSRC of packet sender                     |
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
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |                  profile-specific extensions                  |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

class RtcpPacket {
 public:
  typedef enum {
    UNKNOWN = 0,
    SR = 200,
    RR = 201,
    SDES = 202,
    BYE = 203,
    APP = 204
  } PAYLOAD_TYPE;

 public:
  RtcpPacket();
  RtcpPacket(const uint8_t *buffer, size_t size);
  RtcpPacket(const RtcpPacket &rtp_packet);
  RtcpPacket(RtcpPacket &&rtp_packet);
  RtcpPacket &operator=(const RtcpPacket &rtp_packet);
  RtcpPacket &operator=(RtcpPacket &&rtp_packet);

  ~RtcpPacket();

 public:
  // Set Header
  void SetVerion(uint8_t version) { version_ = version; }
  void SetPadding(uint8_t padding) { padding_ = padding; }
  void SetReceptionReportCount(uint8_t reception_report_count) {
    reception_report_count_ = reception_report_count;
  }
  void SetPayloadType(PAYLOAD_TYPE payload_type) {
    payload_type_ = payload_type;
  }
  void SetLength(uint16_t length) { length_ = length; }

 public:
  typedef struct {
    uint8_t v : 2;
    uint8_t p : 1;
    uint8_t rc : 5;
    uint8_t pt : 8;
    uint16_t length : 16;
  } RTCP_HEADER;

  typedef struct {
    uint32_t ssrc_of_sender : 32;
    uint64_t ntp_timestamp : 64;
    uint32_t rtp_timestamp : 32;
    uint32_t total_sent_count : 32;
    uint32_t total_payload_sent_count : 32;
  } SENDER_INFO;

  typedef struct {
    uint32_t ssrc : 32;
    uint8_t fraction_lost : 8;
    uint32_t cumulative_packets_lost : 24;
    uint32_t highest_sequence_number_received : 32;
    uint32_t jitter : 32;
    uint32_t lsr : 32;
    uint32_t dlsr : 32;
  } REPORT;

  void SetRtcpHeader(RTCP_HEADER &rtcp_header) {
    rtcp_header_.v = rtcp_header.v;
    rtcp_header_.p = rtcp_header.p;
    rtcp_header_.rc = rtcp_header.rc;
    rtcp_header_.pt = rtcp_header.pt;
    rtcp_header_.length = rtcp_header.length;
  }

  void SetSenderInfo(SENDER_INFO &sender_info) {
    sender_info_.ssrc_of_sender = sender_info.ssrc_of_sender;
    sender_info_.ntp_timestamp = sender_info.ntp_timestamp;
    sender_info_.rtp_timestamp = sender_info.rtp_timestamp;
    sender_info_.total_sent_count = sender_info.total_sent_count;
    sender_info_.total_payload_sent_count =
        sender_info.total_payload_sent_count;
  }

  void SetReport(REPORT &report) {
    report_.ssrc = report.ssrc;
    report_.fraction_lost = report.fraction_lost;
    report_.cumulative_packets_lost = report.cumulative_packets_lost;
    report_.highest_sequence_number_received =
        report.highest_sequence_number_received;
    report_.jitter = report.jitter;
    report_.lsr = report.lsr;
    report_.dlsr = report.dlsr;
  }

 public:
  const uint8_t *Encode(uint8_t *payload, size_t payload_size);
  size_t Decode(uint8_t *payload);

 public:
  // Get Header
  const uint8_t Verion() { return version_; }
  const uint8_t Padding() { return padding_; }
  const uint8_t ReceptionReportCount() { return reception_report_count_; }
  const PAYLOAD_TYPE PayloadType() { return PAYLOAD_TYPE(payload_type_); }
  const uint16_t Length() { return length_; }

 private:
  // Header
  uint8_t version_ = 0;
  uint8_t padding_ = false;
  uint8_t reception_report_count_ = 0;
  uint8_t payload_type_ = 0;
  uint16_t length_ = 0;

  RTCP_HEADER rtcp_header_;
  SENDER_INFO sender_info_;
  REPORT report_;
};

#endif