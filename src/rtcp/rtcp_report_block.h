#ifndef _RTCP_REPORT_BLOCK_H_
#define _RTCP_REPORT_BLOCK_H_

// Report block 1
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                 SSRC_1 (SSRC of first source)                 |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | fraction lost |       cumulative number of packets lost       |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           extended highest sequence number received           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                      interarrival jitter                      |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         last SR (LSR)                         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                   delay since last SR (DLSR)                  |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#include <stdint.h>

class RtcpReportBlock {
 public:
  RtcpReportBlock();
  ~RtcpReportBlock();

 public:
  typedef struct {
    uint32_t source_ssrc : 32;
    uint8_t fraction_lost : 8;
    uint32_t cumulative_lost : 24;
    uint32_t extended_high_seq_num : 32;
    uint32_t jitter : 32;
    uint32_t lsr : 32;
    uint32_t dlsr : 32;
  } REPORT;

  void SetReport(REPORT &report) {
    report_.source_ssrc = report.source_ssrc;
    report_.fraction_lost = report.fraction_lost;
    report_.cumulative_lost = report.cumulative_lost;
    report_.extended_high_seq_num = report.extended_high_seq_num;
    report_.jitter = report.jitter;
    report_.lsr = report.lsr;
    report_.dlsr = report.dlsr;
  }

 private:
  REPORT report_;
};

#endif