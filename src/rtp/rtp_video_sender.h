#ifndef _RTP_VIDEO_SENDER_H_
#define _RTP_VIDEO_SENDER_H_

#include <functional>
#include <mutex>
#include <thread>

#include "ringbuffer.h"
#include "rtp_packet.h"
#include "thread_base.h"

class RtpVideoSender : public ThreadBase {
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

  void Start();
  void Stop();

 private:
  bool Process() override;

 private:
  std::function<void(RtpPacket &)> rtp_packet_send_func_ = nullptr;
  RingBuffer<RtpPacket> rtp_packe_queue_;

  bool stop_ = true;
  std::mutex mutex_;
};

#endif