#ifndef _RTP_VIDEO_SENDER_H_
#define _RTP_VIDEO_SENDER_H_

#include <functional>
#include <thread>

#include "ringbuffer.h"
#include "rtp_packet.h"

class RtpVideoSender {
 public:
  RtpVideoSender();
  ~RtpVideoSender();

 public:
  void Enqueue(std::vector<RtpPacket> &rtp_packets);

 public:
  void SetRtpPacketSendFunc(
      std::function<void(RtpPacket &)> rtp_packet_send_func) {
    rtp_packet_send_func_ = rtp_packet_send_func;
  }

 private:
  void Process();

 private:
  std::function<void(RtpPacket &)> rtp_packet_send_func_ = nullptr;
  RingBuffer<RtpPacket> rtp_packe_queue_;
  std::thread *send_thread_ = nullptr;
  bool start_ = false;
};

#endif