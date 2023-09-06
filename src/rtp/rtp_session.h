#ifndef _RTP_SESSION_H_
#define _RTP_SESSION_H_

#include <stdint.h>

#include <vector>

#include "rtp_packet.h"

class RtpSession

{
 public:
  RtpSession(uint32_t payload_type);
  ~RtpSession();

 public:
  RtpPacket Encode(uint8_t* buffer, size_t size);

 private:
  uint32_t version_ = 0;
  bool has_padding_ = false;
  bool has_extension_ = false;
  uint32_t total_csrc_number_ = 0;
  bool marker_ = false;
  uint8_t payload_type_ = 0;
  uint16_t sequence_number_ = 0;
  uint32_t timestamp_ = 0;
  uint32_t ssrc_ = 0;
  std::vector<uint32_t> csrcs_;
  uint16_t profile_ = 0;
  uint16_t extension_profile_ = 0;
  uint16_t extension_len_ = 0;
  uint8_t* extension_data_ = nullptr;

 private:
  RtpPacket* rtp_packet_ = nullptr;
};

#endif