#include "rtp_session.h"

#include <chrono>

#include "log.h"

#define RTP_VERSION 1

RtpSession ::RtpSession(PAYLOAD_TYPE payload_type)
    : version_(RTP_VERSION),
      has_padding_(false),
      has_extension_(false),
      payload_type_(payload_type),
      sequence_number_(0) {}

RtpSession ::~RtpSession() {
  if (extension_data_) {
    delete extension_data_;
    extension_data_ = nullptr;
  }

  // if (rtp_packet_) {
  //   delete rtp_packet_;
  //   rtp_packet_ = nullptr;
  // }
}

void RtpSession::Encode(uint8_t* buffer, size_t size,
                        std::vector<RtpPacket>& packets) {
  // if (!rtp_packet_) {
  //   rtp_packet_ = new RtpPacket();
  // }

  RtpPacket rtp_packet;

  if (size <= MAX_NALU_LEN) {
    rtp_packet.SetVerion(version_);
    rtp_packet.SetHasPadding(has_padding_);
    rtp_packet.SetHasExtension(has_extension_);
    rtp_packet.SetMarker(1);
    rtp_packet.SetPayloadType(PAYLOAD_TYPE(payload_type_));
    rtp_packet.SetSequenceNumber(sequence_number_++);

    timestamp_ =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    rtp_packet.SetTimestamp(timestamp_);
    rtp_packet.SetSsrc(ssrc_);

    if (!csrcs_.empty()) {
      rtp_packet.SetCsrcs(csrcs_);
    }

    if (has_extension_) {
      rtp_packet.SetExtensionProfile(extension_profile_);
      rtp_packet.SetExtensionData(extension_data_, extension_len_);
    }

    RtpPacket::NALU_HEADER nalu_header;
    nalu_header.forbidden_bit = 0;
    nalu_header.nal_reference_idc = 1;
    nalu_header.nal_unit_type = 1;
    rtp_packet.SetNaluHeader(nalu_header);

    rtp_packet.EncodeH264Nalu(buffer, size);
    packets.emplace_back(rtp_packet);

  } else {
    size_t last_packet_size = size % MAX_NALU_LEN;
    size_t packet_num = size / MAX_NALU_LEN + (last_packet_size ? 1 : 0);

    for (size_t index = 0; index * MAX_NALU_LEN < size + MAX_NALU_LEN;
         index++) {
      rtp_packet.SetVerion(version_);
      rtp_packet.SetHasPadding(has_padding_);
      rtp_packet.SetHasExtension(has_extension_);
      rtp_packet.SetMarker(1);
      rtp_packet.SetPayloadType(PAYLOAD_TYPE(payload_type_));
      rtp_packet.SetSequenceNumber(sequence_number_++);

      timestamp_ =
          std::chrono::high_resolution_clock::now().time_since_epoch().count();
      rtp_packet.SetTimestamp(timestamp_);
      rtp_packet.SetSsrc(ssrc_);

      if (!csrcs_.empty()) {
        rtp_packet.SetCsrcs(csrcs_);
      }

      if (has_extension_) {
        rtp_packet.SetExtensionProfile(extension_profile_);
        rtp_packet.SetExtensionData(extension_data_, extension_len_);
      }

      rtp_packet.Encode(buffer, size);
      packets.emplace_back(rtp_packet);
    }
  }
}

size_t RtpSession::Decode(RtpPacket& packet, uint8_t* payload) {
  return packet.DecodeH264Nalu(payload);
}