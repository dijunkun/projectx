#ifndef _RTP_PACKET_H_
#define _RTP_PACKET_H_

#include <cstring>
#include <vector>

//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |V=2|P|X|  CC   |M|     PT      |       sequence number         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           timestamp                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           synchronization source (SSRC) identifier            |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |            Contributing source (CSRC) identifiers             |
// |                             ....                              |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |       defined by profile      |            length             |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          Extensions                           |
// |                             ....                              |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |                           Payload                             |
// |             ....              :  padding...                   |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |               padding         | Padding size  |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

class RtpPacket {
 public:
  RtpPacket();
  RtpPacket(const uint8_t *buffer, size_t size);
  RtpPacket(const RtpPacket &rtp_packet);
  RtpPacket &operator=(const RtpPacket &rtp_packet);
  RtpPacket(RtpPacket &&rtp_packet);

  ~RtpPacket();

 public:
  // Set Header
  void SetMarker(bool has_marker) { marker_ = has_marker; }
  void SetPayloadType(uint8_t payload_type) { payload_type_ = payload_type; }
  void SetSequenceNumber(uint16_t sequence_number) {
    sequence_number_ = sequence_number;
  }
  void SetTimestamp(uint32_t timestamp) { timestamp_ = timestamp; }
  void SetSsrc(uint32_t ssrc) { ssrc_ = ssrc; }
  void SetCsrcs(){};

  void SetHasExtension(bool has_extension) { has_extension_ = has_extension; };
  void SetExtensionProfile(uint16_t extension_profile) {
    extension_profile_ = extension_profile;
  }
  void SetExtensionData(uint8_t *extension_data, size_t extension_len) {
    extension_len_ = extension_len;
    extension_data_ = new uint8_t[extension_len_];
    memcpy(extension_data_, extension_data, extension_len_);
  }

 public:
  const uint8_t *Encode(uint8_t *payload, size_t payload_size);
  const uint8_t *Decode();

 public:
  // Get Header
  bool Marker() const { return marker_; }
  uint8_t PayloadType() const { return payload_type_; }
  uint16_t SequenceNumber() const { return sequence_number_; }
  uint32_t Timestamp() const { return timestamp_; }
  uint32_t Ssrc() const { return ssrc_; }
  std::vector<uint32_t> Csrcs() const;
  bool HasExtension() const { return has_extension_; };
  uint16_t ExtensionProfile() const { return extension_profile_; }
  uint8_t *ExtensionData() const { return extension_data_; }

  // Payload
  uint8_t *Payload() const { return payload_; };
  size_t PayloadSize() const { return payload_size_; }

 public:
  const uint8_t *Buffer() { return buffer_; }
  const size_t Size() { return size_; }

 private:
  // Header
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
  uint8_t *extension_data_ = nullptr;

  // Payload
  uint8_t *payload_ = nullptr;
  size_t payload_size_ = 0;

  // Entire RTP buffer
  uint8_t *buffer_ = nullptr;
  size_t size_ = 0;
};

#endif