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
// |  header eXtension profile id  |       length in 32bits        |
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
  RtpPacket(const uint8_t *buffer, size_t size);
  RtpPacket(const RtpPacket &rhs);
  RtpPacket() {}

  ~RtpPacket();

 public:
  // Header
  void SetMarker(bool marker) { marker_ = marker; }
  void SetPayloadType(uint8_t payload_type) { payload_type_ = payload_type; }
  void SetSequenceNumber(uint16_t sequence_number) {
    sequence_number_ = sequence_number;
  }
  void SetTimestamp(uint32_t timestamp) { timestamp_ = timestamp; }
  void SetSsrc(uint32_t ssrc) { ssrc_ = ssrc; }
  void SetCsrcs(){};
  void SetHeadersSize(size_t payload_offset) {
    payload_offset_ = payload_offset;
  }
  void SetHasExtension(bool has_extension) { has_extension_ = has_extension; };
  void SetExtensionID(uint16_t extension_id) { extension_id_ = extension_id; }
  void SetExtensionData(uint8_t *extension_data) {
    extension_data_ = extension_data;
  }

  // Payload
  void SetPayload(uint8_t *payload) {
    payload_ = new uint8_t[payload_size_];
    memcpy(payload_, payload, payload_size_);
  };
  void SetPayloadSize(size_t payload_size) { payload_size_ = payload_size; };
  void SetPaddingSize(size_t padding_size) { padding_size_ = padding_size; }

  // Header
  bool Marker() const { return marker_; }
  uint8_t PayloadType() const { return payload_type_; }
  uint16_t SequenceNumber() const { return sequence_number_; }
  uint32_t Timestamp() const { return timestamp_; }
  uint32_t Ssrc() const { return ssrc_; }
  std::vector<uint32_t> Csrcs() const;
  size_t HeadersSize() const { return payload_offset_; }
  bool HasExtension() const { return has_extension_; };
  uint16_t ExtensionID() const { return extension_id_; }
  uint8_t *ExtensionData() const { return extension_data_; }

  // Payload
  uint8_t *Payload() const { return payload_; };
  size_t PayloadSize() const { return payload_size_; }
  size_t PaddingSize() const { return padding_size_; }

 private:
  // Header
  bool marker_ = false;
  uint8_t payload_type_ = 0;
  uint16_t sequence_number_ = 0;
  uint32_t timestamp_ = 0;
  uint32_t ssrc_ = 0;
  size_t payload_offset_ = 0;
  bool has_extension_ = false;
  uint16_t extension_id_ = 0;
  uint8_t *extension_data_ = nullptr;

  // Payload
  uint8_t *payload_ = nullptr;
  size_t payload_size_ = 0;
  uint8_t padding_size_;
};

#endif