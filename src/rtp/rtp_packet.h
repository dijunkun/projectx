#ifndef _RTP_PACKET_H_
#define _RTP_PACKET_H_

#include <stdint.h>

#include <vector>

// Common
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

// H264
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
// |  FU indicator |   FU header   |                               |
// |                                                               |
// |                           FU Payload                          |
// |                                                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |               padding         | Padding size  |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// |  FU indicator |   FU header   |
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |F|NRI|   Type  |S|E|R|   Type  |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#define MAX_NALU_LEN 1400
typedef enum { H264 = 96, OPUS = 97, USER_DEFINED = 127 } PAYLOAD_TYPE;

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
  void SetVerion(uint32_t version) { version_ = version; }
  void SetHasPadding(bool has_padding) { has_padding_ = has_padding; }
  void SetHasExtension(bool has_extension) { has_extension_ = has_extension; }
  void SetMarker(bool marker) { marker_ = marker; }
  void SetPayloadType(PAYLOAD_TYPE payload_type) {
    payload_type_ = payload_type;
  }
  void SetSequenceNumber(uint16_t sequence_number) {
    sequence_number_ = sequence_number;
  }
  void SetTimestamp(uint32_t timestamp) { timestamp_ = timestamp; }
  void SetSsrc(uint32_t ssrc) { ssrc_ = ssrc; }
  void SetCsrcs(std::vector<uint32_t> &csrcs) { csrcs_ = csrcs; }

  void SetExtensionProfile(uint16_t extension_profile) {
    extension_profile_ = extension_profile;
  }
  void SetExtensionData(uint8_t *extension_data, size_t extension_len) {
    extension_len_ = extension_len;
    extension_data_ = new uint8_t[extension_len_];
    memcpy(extension_data_, extension_data, extension_len_);
  }

 public:
  typedef struct {
    unsigned char forbidden_bit : 1;
    unsigned char nal_reference_idc : 2;
    unsigned char nal_unit_type : 5;
  } NALU_HEADER;

  typedef struct {
    unsigned char f : 1;
    unsigned char nri : 2;
    unsigned char type : 5;
  } FU_INDICATOR;

  typedef struct {
    unsigned char s : 1;
    unsigned char e : 1;
    unsigned char r : 1;
    unsigned char type : 5;
  } FU_HEADER;

  void SetNaluHeader(NALU_HEADER nalu_header) {
    nalu_header_.forbidden_bit = nalu_header.forbidden_bit;
    nalu_header_.nal_reference_idc = nalu_header.nal_reference_idc;
    nalu_header_.nal_unit_type = nalu_header.nal_unit_type;
  }

  void FuAHeader(FU_INDICATOR fu_indicator, FU_HEADER fu_header) {
    fu_indicator_.f = fu_indicator.f;
    fu_indicator_.nri = fu_indicator.nri;
    fu_indicator_.type = fu_indicator.nri;

    fu_header_.s = fu_header.s;
    fu_header_.e = fu_header.e;
    fu_header_.r = fu_header.r;
    fu_header_.type = fu_header.type;
  }

 public:
  const uint8_t *Encode(uint8_t *payload, size_t payload_size);
  const uint8_t *EncodeH264Nalu(uint8_t *payload, size_t payload_size);
  const uint8_t *Decode();
  size_t DecodeH264Nalu(uint8_t *payload);

 public:
  // Get Header
  const uint32_t Verion() { return version_; }
  const bool HasPadding() { return has_padding_; }
  const bool HasExtension() { return has_extension_; }
  const bool Marker() { return marker_; }
  const uint32_t PayloadType() { return payload_type_; }
  const uint16_t SequenceNumber() { return sequence_number_; }
  const uint32_t Timestamp() { return timestamp_; }
  const uint32_t Ssrc() { return ssrc_; }
  const std::vector<uint32_t> Csrcs() { return csrcs_; };
  const uint16_t ExtensionProfile() { return extension_profile_; }
  const uint8_t *ExtensionData() { return extension_data_; }

  // Payload
  const uint8_t *Payload() { return payload_; };
  const size_t PayloadSize() { return payload_size_; }

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
  uint32_t payload_type_ = 0;
  uint16_t sequence_number_ = 0;
  uint32_t timestamp_ = 0;
  uint32_t ssrc_ = 0;
  std::vector<uint32_t> csrcs_;
  uint16_t profile_ = 0;
  uint16_t extension_profile_ = 0;
  uint16_t extension_len_ = 0;
  uint8_t *extension_data_ = nullptr;
  NALU_HEADER nalu_header_;
  FU_INDICATOR fu_indicator_;
  FU_HEADER fu_header_;

  // Payload
  uint8_t *payload_ = nullptr;
  size_t payload_size_ = 0;

  // Entire RTP buffer
  uint8_t *buffer_ = nullptr;
  size_t size_ = 0;
};

#endif