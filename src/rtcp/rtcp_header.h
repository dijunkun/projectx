#ifndef _RTCP_HEADER_H_
#define _RTCP_HEADER_H_

#include <stdint.h>

#include "log.h"

// RTCP header
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |V=2|P|   RC    |   PT=SR=200   |            length             |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#define DEFAULT_RTCP_VERSION 2
#define DEFAULT_RTCP_HEADER_SIZE 4

class RtcpHeader {
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
  RtcpHeader();
  RtcpHeader(const uint8_t* buffer, uint32_t size);
  ~RtcpHeader();

 public:
  void SetVerion(uint8_t version) { version_ = version; }
  void SetPadding(uint8_t padding) { padding_ = padding; }
  void SetCountOrFormat(uint8_t count_or_format) {
    count_or_format_ = count_or_format;
  }
  void SetPayloadType(PAYLOAD_TYPE payload_type) {
    payload_type_ = payload_type;
  }
  void SetLength(uint16_t length) { length_ = length; }

 public:
  uint8_t Verion() const { return version_; }
  uint8_t Padding() const { return padding_; }
  uint8_t CountOrFormat() const { return count_or_format_; }
  PAYLOAD_TYPE PayloadType() const {
    return PAYLOAD_TYPE((uint8_t)payload_type_);
  }
  uint16_t Length() const { return length_; }

  int Encode(uint8_t version, uint8_t padding, uint8_t count_or_format,
             uint8_t payload_type, uint16_t length, uint8_t* buffer);

 private:
  uint8_t version_ : 2;
  uint8_t padding_ : 1;
  uint8_t count_or_format_ : 5;
  PAYLOAD_TYPE payload_type_ : 8;
  uint16_t length_ : 16;
};

#endif