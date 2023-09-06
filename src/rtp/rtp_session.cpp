#include "rtp_session.h"

#include <chrono>

#define RTP_VERSION 1

RtpSession ::RtpSession(uint32_t payload_type)
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

  if (rtp_packet_) {
    delete rtp_packet_;
    rtp_packet_ = nullptr;
  }
}

RtpPacket RtpSession::Encode(uint8_t* buffer, size_t size) {
  if (!rtp_packet_) {
    rtp_packet_ = new RtpPacket();
  }

  rtp_packet_->SetVerion(version_);
  rtp_packet_->SetHasPadding(has_padding_);
  rtp_packet_->SetHasExtension(has_extension_);
  rtp_packet_->SetMarker(marker_);
  rtp_packet_->SetPayloadType(payload_type_);
  rtp_packet_->SetSequenceNumber(sequence_number_++);

  timestamp_ =
      std::chrono::high_resolution_clock::now().time_since_epoch().count();
  rtp_packet_->SetTimestamp(timestamp_);
  rtp_packet_->SetSsrc(ssrc_);

  if (!csrcs_.empty()) {
    rtp_packet_->SetCsrcs(csrcs_);
  }

  if (has_extension_) {
    rtp_packet_->SetExtensionProfile(extension_profile_);
    rtp_packet_->SetExtensionData(extension_data_, extension_len_);
  }

  rtp_packet_->Encode(buffer, size);

  return *rtp_packet_;
}