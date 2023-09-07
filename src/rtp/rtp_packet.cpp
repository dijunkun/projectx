#include "rtp_packet.h"

#include <string>

#include "log.h"

RtpPacket::RtpPacket() : buffer_(new uint8_t[DEFAULT_MTU]), size_(DEFAULT_MTU) {
  memset(buffer_, 0, DEFAULT_MTU);
}

RtpPacket::RtpPacket(const uint8_t *buffer, size_t size) {
  if (size > 0) {
    buffer_ = new uint8_t[size];
    memcpy(buffer_, buffer, size);
    size_ = size;
  }
}

RtpPacket::RtpPacket(const RtpPacket &rtp_packet) {
  if (rtp_packet.size_ > 0) {
    buffer_ = new uint8_t[rtp_packet.size_];
    memcpy(buffer_, rtp_packet.buffer_, rtp_packet.size_);
    size_ = rtp_packet.size_;
  }
}

RtpPacket::RtpPacket(RtpPacket &&rtp_packet)
    : buffer_((uint8_t *)std::move(rtp_packet.buffer_)),
      size_(rtp_packet.size_) {
  rtp_packet.buffer_ = nullptr;
  rtp_packet.size_ = 0;
}

RtpPacket &RtpPacket::operator=(const RtpPacket &rtp_packet) {
  if (&rtp_packet != this) {
    if (buffer_) {
      delete buffer_;
      buffer_ = nullptr;
    }
    buffer_ = new uint8_t[rtp_packet.size_];
    memcpy(buffer_, rtp_packet.buffer_, rtp_packet.size_);
    size_ = rtp_packet.size_;
  }
  return *this;
}

RtpPacket &RtpPacket::operator=(RtpPacket &&rtp_packet) {
  if (&rtp_packet != this) {
    buffer_ = std::move(rtp_packet.buffer_);
    rtp_packet.buffer_ = nullptr;
    size_ = rtp_packet.size_;
    rtp_packet.size_ = 0;
  }
  return *this;
}

RtpPacket::~RtpPacket() {
  if (buffer_) {
    delete buffer_;
    buffer_ = nullptr;
  }
  size_ = 0;

  if (extension_data_) {
    delete extension_data_;
    extension_data_ = nullptr;
  }
  extension_len_ = 0;
  payload_size_ = 0;
}

const uint8_t *RtpPacket::Encode(uint8_t *payload, size_t payload_size) {
  buffer_[0] = (version_ << 6) | (has_padding_ << 5) | (has_extension_ << 4) |
               total_csrc_number_;
  buffer_[1] = (marker_ << 7) | payload_type_;
  buffer_[2] = (sequence_number_ >> 8) & 0xFF;
  buffer_[3] = sequence_number_ & 0xFF;
  buffer_[4] = (timestamp_ >> 24) & 0xFF;
  buffer_[5] = (timestamp_ >> 16) & 0xFF;
  buffer_[6] = (timestamp_ >> 8) & 0xFF;
  buffer_[7] = timestamp_ & 0xFF;
  buffer_[8] = (ssrc_ >> 24) & 0xFF;
  buffer_[9] = (ssrc_ >> 16) & 0xFF;
  buffer_[10] = (ssrc_ >> 8) & 0xFF;
  buffer_[11] = ssrc_ & 0xFF;

  for (uint32_t index = 0; index < total_csrc_number_ && !csrcs_.empty();
       index++) {
    buffer_[12 + index] = (csrcs_[index] >> 24) & 0xFF;
    buffer_[13 + index] = (csrcs_[index] >> 16) & 0xFF;
    buffer_[14 + index] = (csrcs_[index] >> 8) & 0xFF;
    buffer_[15 + index] = csrcs_[index] & 0xFF;
  }

  uint32_t extension_offset =
      total_csrc_number_ && !csrcs_.empty() ? total_csrc_number_ * 4 : 0;
  if (has_extension_ && extension_data_) {
    buffer_[12 + extension_offset] = extension_profile_ >> 8;
    buffer_[13 + extension_offset] = extension_profile_ & 0xff;
    buffer_[14 + extension_offset] = (extension_len_ >> 8) & 0xFF;
    buffer_[15 + extension_offset] = extension_len_ & 0xFF;
    memcpy(buffer_ + 16 + extension_offset, extension_data_, extension_len_);
  }

  uint32_t payload_offset =
      (has_extension_ && extension_data_ ? extension_len_ : 0) +
      extension_offset;

  memcpy(buffer_ + 12 + payload_offset, payload, payload_size);
  size_ = payload_size + (12 + payload_offset);

  return buffer_;
}

const uint8_t *RtpPacket::EncodeH264Nalu(uint8_t *payload,
                                         size_t payload_size) {
  buffer_[0] = (version_ << 6) | (has_padding_ << 5) | (has_extension_ << 4) |
               total_csrc_number_;
  buffer_[1] = (marker_ << 7) | payload_type_;
  buffer_[2] = (sequence_number_ >> 8) & 0xFF;
  buffer_[3] = sequence_number_ & 0xFF;
  buffer_[4] = (timestamp_ >> 24) & 0xFF;
  buffer_[5] = (timestamp_ >> 16) & 0xFF;
  buffer_[6] = (timestamp_ >> 8) & 0xFF;
  buffer_[7] = timestamp_ & 0xFF;
  buffer_[8] = (ssrc_ >> 24) & 0xFF;
  buffer_[9] = (ssrc_ >> 16) & 0xFF;
  buffer_[10] = (ssrc_ >> 8) & 0xFF;
  buffer_[11] = ssrc_ & 0xFF;

  for (uint32_t index = 0; index < total_csrc_number_ && !csrcs_.empty();
       index++) {
    buffer_[12 + index] = (csrcs_[index] >> 24) & 0xFF;
    buffer_[13 + index] = (csrcs_[index] >> 16) & 0xFF;
    buffer_[14 + index] = (csrcs_[index] >> 8) & 0xFF;
    buffer_[15 + index] = csrcs_[index] & 0xFF;
  }

  uint32_t extension_offset =
      total_csrc_number_ && !csrcs_.empty() ? total_csrc_number_ * 4 : 0;
  if (has_extension_ && extension_data_) {
    buffer_[12 + extension_offset] = extension_profile_ >> 8;
    buffer_[13 + extension_offset] = extension_profile_ & 0xff;
    buffer_[14 + extension_offset] = (extension_len_ >> 8) & 0xFF;
    buffer_[15 + extension_offset] = extension_len_ & 0xFF;
    memcpy(buffer_ + 16 + extension_offset, extension_data_, extension_len_);
  }

  uint32_t payload_offset =
      (has_extension_ && extension_data_ ? extension_len_ : 0) +
      extension_offset;

  buffer_[12 + payload_offset] = nalu_header_.forbidden_bit << 7 |
                                 nalu_header_.nal_reference_idc << 6 |
                                 nalu_header_.nal_unit_type;

  memcpy(buffer_ + 13 + payload_offset, payload, payload_size);
  size_ = payload_size + (13 + payload_offset);

  return buffer_;
}

const uint8_t *RtpPacket::Decode() {
  version_ = (buffer_[0] >> 6) & 0x03;
  has_padding_ = (buffer_[0] >> 5) & 0x01;
  has_extension_ = (buffer_[0] >> 4) & 0x01;
  total_csrc_number_ = buffer_[0] & 0x0f;
  marker_ = (buffer_[1] >> 7) & 0x01;
  payload_type_ = buffer_[1] & 0x7f;
  sequence_number_ = (buffer_[2] << 8) | buffer_[3];
  timestamp_ =
      (buffer_[4] << 24) | (buffer_[5] << 16) | (buffer_[6] << 8) | buffer_[7];
  ssrc_ = (buffer_[8] << 24) | (buffer_[9] << 16) | (buffer_[10] << 8) |
          buffer_[11];

  for (uint32_t index = 0; index < total_csrc_number_; index++) {
    uint32_t csrc = (buffer_[12 + index] << 24) | (buffer_[13 + index] << 16) |
                    (buffer_[14 + index] << 8) | buffer_[15 + index];
    csrcs_.push_back(csrc);
  }

  uint32_t extension_offset = total_csrc_number_ * 4;
  if (has_extension_) {
    extension_profile_ =
        (buffer_[12 + extension_offset] << 8) | buffer_[13 + extension_offset];
    extension_len_ =
        (buffer_[14 + extension_offset] << 8) | buffer_[15 + extension_offset];

    // extension_data_ = new uint8_t[extension_len_];
    // memcpy(extension_data_, buffer_ + 16 + extension_offset, extension_len_);
    extension_data_ = buffer_ + 16 + extension_offset;
  }

  uint32_t payload_offset =
      (has_extension_ ? extension_len_ : 0) + extension_offset;

  payload_size_ = size_ - (12 + payload_offset);
  // payload_ = new uint8_t[payload_size_];
  // memcpy(payload_, buffer_ + 12 + payload_offset, payload_size_);
  payload_ = buffer_ + 12 + payload_offset;

  return payload_;
}

size_t RtpPacket::DecodeH264Nalu(uint8_t *payload) {
  version_ = (buffer_[0] >> 6) & 0x03;
  has_padding_ = (buffer_[0] >> 5) & 0x01;
  has_extension_ = (buffer_[0] >> 4) & 0x01;
  total_csrc_number_ = buffer_[0] & 0x0f;
  marker_ = (buffer_[1] >> 7) & 0x01;
  payload_type_ = buffer_[1] & 0x7f;
  sequence_number_ = (buffer_[2] << 8) | buffer_[3];
  timestamp_ =
      (buffer_[4] << 24) | (buffer_[5] << 16) | (buffer_[6] << 8) | buffer_[7];
  ssrc_ = (buffer_[8] << 24) | (buffer_[9] << 16) | (buffer_[10] << 8) |
          buffer_[11];

  for (uint32_t index = 0; index < total_csrc_number_; index++) {
    uint32_t csrc = (buffer_[12 + index] << 24) | (buffer_[13 + index] << 16) |
                    (buffer_[14 + index] << 8) | buffer_[15 + index];
    csrcs_.push_back(csrc);
  }

  uint32_t extension_offset = total_csrc_number_ * 4;
  if (has_extension_) {
    extension_profile_ =
        (buffer_[12 + extension_offset] << 8) | buffer_[13 + extension_offset];
    extension_len_ =
        (buffer_[14 + extension_offset] << 8) | buffer_[15 + extension_offset];

    // extension_data_ = new uint8_t[extension_len_];
    // memcpy(extension_data_, buffer_ + 16 + extension_offset, extension_len_);
    extension_data_ = buffer_ + 16 + extension_offset;
  }

  uint32_t payload_offset =
      (has_extension_ ? extension_len_ : 0) + extension_offset;

  nalu_header_.forbidden_bit = (buffer_[12 + payload_offset] >> 7) & 0x01;
  nalu_header_.nal_reference_idc = (buffer_[12 + payload_offset] >> 6) & 0x01;
  nalu_header_.nal_unit_type = (buffer_[12 + payload_offset]) & 0x31;

  payload_size_ = size_ - (13 + payload_offset);
  payload_ = buffer_ + 13 + payload_offset;
  memcpy(payload, payload_, payload_size_);
  return payload_size_;
}