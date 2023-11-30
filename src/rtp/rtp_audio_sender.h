/*
 * @Author: DI JUNKUN
 * @Date: 2023-11-24
 * Copyright (c) 2023 by DI JUNKUN, All Rights Reserved.
 */

#ifndef _RTP_AUDIO_SENDER_H_
#define _RTP_AUDIO_SENDER_H_

#include <functional>

#include "ringbuffer.h"
#include "rtcp_sender_report.h"
#include "rtp_packet.h"
#include "rtp_statistics.h"
#include "thread_base.h"

class RtpAudioSender : public ThreadBase {
 public:
  RtpAudioSender();
  ~RtpAudioSender();

 public:
  void Enqueue(std::vector<RtpPacket> &rtp_packets);
  void SetSendDataFunc(std::function<int(const char *, size_t)> data_send_func);

 private:
 private:
  int SendRtpPacket(RtpPacket &rtp_packet);
  int SendRtcpSR(RtcpSenderReport &rtcp_sr);

  bool CheckIsTimeSendSR();

 private:
  bool Process() override;

 private:
  std::function<int(const char *, size_t)> data_send_func_ = nullptr;
  RingBuffer<RtpPacket> rtp_packe_queue_;
  std::unique_ptr<RtpStatistics> rtp_statistics_ = nullptr;
  uint32_t last_send_bytes_ = 0;
  uint32_t last_send_rtcp_sr_packet_ts_ = 0;
  uint32_t total_rtp_packets_sent_ = 0;
  uint32_t total_rtp_payload_sent_ = 0;
};

#endif