#include "rtp_codec.h"

#include <chrono>

#include "log.h"

#define RTP_VERSION 2
#define NALU 1
#define FU_A 28
#define FU_B 29

RtpCodec ::RtpCodec(RtpPacket::PAYLOAD_TYPE payload_type)
    : version_(RTP_VERSION),
      has_padding_(false),
      has_extension_(false),
      payload_type_(payload_type),
      sequence_number_(0) {}

RtpCodec ::~RtpCodec() {
  if (extension_data_) {
    delete extension_data_;
    extension_data_ = nullptr;
  }

  // if (rtp_packet_) {
  //   delete rtp_packet_;
  //   rtp_packet_ = nullptr;
  // }
}

void RtpCodec::Encode(uint8_t* buffer, size_t size,
                      std::vector<RtpPacket>& packets) {
  // if (!rtp_packet_) {
  //   rtp_packet_ = new RtpPacket();
  // }

  RtpPacket rtp_packet;

  if (RtpPacket::PAYLOAD_TYPE::H264 == payload_type_) {
    if (size <= MAX_NALU_LEN) {
      rtp_packet.SetVerion(version_);
      rtp_packet.SetHasPadding(has_padding_);
      rtp_packet.SetHasExtension(has_extension_);
      rtp_packet.SetMarker(1);
      rtp_packet.SetPayloadType(RtpPacket::PAYLOAD_TYPE(payload_type_));
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

      RtpPacket::FU_INDICATOR fu_indicator;
      fu_indicator.forbidden_bit = 0;
      fu_indicator.nal_reference_idc = 1;
      fu_indicator.nal_unit_type = NALU;
      rtp_packet.SetFuIndicator(fu_indicator);

      rtp_packet.EncodeH264Nalu(buffer, size);
      packets.emplace_back(rtp_packet);

    } else {
      size_t last_packet_size = size % MAX_NALU_LEN;
      size_t packet_num = size / MAX_NALU_LEN + (last_packet_size ? 1 : 0);

      for (size_t index = 0; index < packet_num; index++) {
        rtp_packet.SetVerion(version_);
        rtp_packet.SetHasPadding(has_padding_);
        rtp_packet.SetHasExtension(has_extension_);
        rtp_packet.SetMarker(index == packet_num ? 1 : 0);
        rtp_packet.SetPayloadType(RtpPacket::PAYLOAD_TYPE(payload_type_));
        rtp_packet.SetSequenceNumber(sequence_number_++);

        timestamp_ = std::chrono::high_resolution_clock::now()
                         .time_since_epoch()
                         .count();
        rtp_packet.SetTimestamp(timestamp_);
        rtp_packet.SetSsrc(ssrc_);

        if (!csrcs_.empty()) {
          rtp_packet.SetCsrcs(csrcs_);
        }

        if (has_extension_) {
          rtp_packet.SetExtensionProfile(extension_profile_);
          rtp_packet.SetExtensionData(extension_data_, extension_len_);
        }

        RtpPacket::FU_INDICATOR fu_indicator;
        fu_indicator.forbidden_bit = 0;
        fu_indicator.nal_reference_idc = 0;
        fu_indicator.nal_unit_type = FU_A;

        RtpPacket::FU_HEADER fu_header;
        fu_header.start = index == 0 ? 1 : 0;
        fu_header.end = index == packet_num - 1 ? 1 : 0;
        fu_header.remain_bit = 0;
        fu_header.nal_unit_type = FU_A;

        rtp_packet.SetFuIndicator(fu_indicator);
        rtp_packet.SetFuHeader(fu_header);

        if (index == packet_num - 1 && last_packet_size > 0) {
          rtp_packet.EncodeH264Fua(buffer + index * MAX_NALU_LEN,
                                   last_packet_size);
        } else {
          rtp_packet.EncodeH264Fua(buffer + index * MAX_NALU_LEN, MAX_NALU_LEN);
        }
        packets.emplace_back(rtp_packet);
      }
    }
  } else if (RtpPacket::PAYLOAD_TYPE::DATA == payload_type_) {
    rtp_packet.SetVerion(version_);
    rtp_packet.SetHasPadding(has_padding_);
    rtp_packet.SetHasExtension(has_extension_);
    rtp_packet.SetMarker(1);
    rtp_packet.SetPayloadType(RtpPacket::PAYLOAD_TYPE(payload_type_));
    rtp_packet.SetSequenceNumber(sequence_number_++);

    timestamp_ =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    rtp_packet.SetTimestamp(timestamp_);
    rtp_packet.SetSsrc(ssrc_);

    rtp_packet.Encode(buffer, size);
    packets.emplace_back(rtp_packet);
  }
}

size_t RtpCodec::Decode(RtpPacket& packet, uint8_t* payload) {
  // if ((packet.Buffer()[13] >> 6) & 0x01) {
  //   LOG_ERROR("End bit!!!!!!!!!!!!!!!");
  // }

  // if ((packet.Buffer()[13] >> 7) & 0x01) {
  //   LOG_ERROR("Start bit!!!!!!!!!!!!!!!");
  // }
  auto nal_unit_type = packet.Buffer()[12] & 0x1F;

  if (NALU == nal_unit_type) {
    LOG_ERROR("Nalu");
    return packet.DecodeH264Nalu(payload);
  } else if (FU_A == nal_unit_type) {
    LOG_ERROR("Fua");
    return packet.DecodeH264Fua(payload);
  } else {
    LOG_ERROR("Default");
    return packet.DecodeData(payload);
  }
}