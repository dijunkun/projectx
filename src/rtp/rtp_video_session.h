#ifndef _RTP_VIDEO_SESSION_H_
#define _RTP_VIDEO_SESSION_H_

#include <stdint.h>

#include <vector>

#include "rtp_packet.h"

class RtpVideoSession {
 public:
  RtpVideoSession(PAYLOAD_TYPE payload_type);
  ~RtpVideoSession();

 public:
  void Encode(uint8_t* buffer, size_t size, std::vector<RtpPacket>& packets);
  size_t Decode(RtpPacket& packet, uint8_t* payload);

  //  protected:
  //   void OnReceiveFrame(uint8_t* payload) = 0;

 private:
  uint32_t version_ = 0;
  bool has_padding_ = false;
  bool has_extension_ = false;
  uint32_t total_csrc_number_ = 0;
  bool marker_ = false;
  uint32_t payload_type_ = 0;
  uint16_t sequence_number_ = 0;
  uint32_t timestamp_ = 0;
  uint32_t ssrc_ = 0;
  std::vector<uint32_t> csrcs_;
  uint16_t profile_ = 0;
  uint16_t extension_profile_ = 0;
  uint16_t extension_len_ = 0;
  uint8_t* extension_data_ = nullptr;

 private:
  // RtpPacket* rtp_packet_ = nullptr;
  RtpPacket::FU_INDICATOR fu_indicator_;
};

#endif