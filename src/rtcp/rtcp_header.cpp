#include "rtcp_header.h"

RtcpHeader::RtcpHeader()
    : version_(0), padding_(0), count_or_format_(0), length_(0) {}

RtcpHeader::~RtcpHeader() {}

int RtcpHeader::Encode(uint8_t version, uint8_t padding,
                       uint8_t count_or_format, uint8_t payload_type,
                       uint16_t length, uint8_t* buffer) {
  if (!buffer) {
    return 0;
  }

  buffer[0] = (version << 6) | (padding << 5) | (count_or_format << 4);
  buffer[1] = payload_type;
  buffer[2] = length >> 8 & 0xFF;
  buffer[3] = length & 0xFF;
  return 4;
}