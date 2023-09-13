#ifndef _RTCP_TYPEDEF_H_
#define _RTCP_TYPEDEF_H_

#include <stdint.h>

#define DEFAULT_RTCP_VERSION 2
#define DEFAULT_RTCP_HEADER_SIZE 4

#define DEFAULT_SR_BLOCK_NUM 1
#define DEFAULT_SR_SIZE 52
#define DEFAULT_RR_BLOCK_NUM 1
#define DEFAULT_RR_SIZE 32

typedef enum {
  UNKNOWN = 0,
  SR = 200,
  RR = 201,
  SDES = 202,
  BYE = 203,
  APP = 204
} RTCP_TYPE;

typedef struct {
  uint32_t sender_ssrc : 32;
  uint64_t ntp_ts_msw : 64;
  uint64_t ntp_ts_lsw : 64;
  uint32_t rtp_ts : 32;
  uint32_t sender_packet_count : 32;
  uint32_t sender_octet_count : 32;
} SenderInfo;

typedef struct {
  uint32_t source_ssrc : 32;
  uint8_t fraction_lost : 8;
  uint32_t cumulative_lost : 24;
  uint32_t extended_high_seq_num : 32;
  uint32_t jitter : 32;
  uint32_t lsr : 32;
  uint32_t dlsr : 32;
} RtcpReportBlock;

#endif