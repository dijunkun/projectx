#ifndef _RTP_VIDEO_SENDER_H_
#define _RTP_VIDEO_SENDER_H_

#include <functional>

#include "ringbuffer.h"
#include "rtcp_packet.h"
#include "rtcp_sender_report.h"
#include "rtp_packet.h"
#include "rtp_video_send_statistics.h"
#include "thread_base.h"

class RtpVideoSender : public ThreadBase {
 public:
  RtpVideoSender();
  ~RtpVideoSender();

 public:
  void Enqueue(std::vector<RtpPacket> &rtp_packets);

 public:
  void SetUdpSender(
      std::function<int(const char *, size_t)> rtp_packet_send_func);

 private:
  int SendRtpPacket(RtpPacket &rtp_packet);
  int SendRtcpSR(RtcpSenderReport &rtcp_sr);

  bool CheckIsTimeSendRtcpPacket();

 private:
  bool Process() override;

 private:
  std::function<int(const char *, size_t)> udp_sender_ = nullptr;
  RingBuffer<RtpPacket> rtp_packe_queue_;

 private:
  std::unique_ptr<RtpVideoSendStatistics> rtp_video_send_statistics_ = nullptr;
  uint32_t last_send_bytes_ = 0;
  uint32_t last_send_rtcp_packet_ts_ = 0;
  uint32_t total_rtp_packets_sent_ = 0;
  uint32_t total_rtp_payload_sent_ = 0;
};

#endif