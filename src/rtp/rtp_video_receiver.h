#ifndef _RTP_VIDEO_RECEIVER_H_
#define _RTP_VIDEO_RECEIVER_H_

#include <functional>
#include <map>
#include <queue>

#include "frame.h"
#include "ringbuffer.h"
#include "rtcp_receiver_report.h"
#include "rtp_codec.h"
#include "rtp_statistics.h"
#include "thread_base.h"

class RtpVideoReceiver : public ThreadBase {
 public:
  RtpVideoReceiver();
  ~RtpVideoReceiver();

 public:
  void InsertRtpPacket(RtpPacket& rtp_packet);

  void SetSendDataFunc(std::function<int(const char*, size_t)> data_send_func);

  void SetOnReceiveCompleteFrame(
      std::function<void(VideoFrame&)> on_receive_complete_frame) {
    on_receive_complete_frame_ = on_receive_complete_frame;
  }

 private:
  bool CheckIsFrameCompleted(RtpPacket& rtp_packet);
  bool CheckIsTimeSendRR();
  int SendRtcpRR(RtcpReceiverReport& rtcp_rr);

 private:
  bool Process() override;

 private:
  std::map<uint16_t, RtpPacket> incomplete_frame_list_;
  uint8_t* nv12_data_ = nullptr;
  std::function<void(VideoFrame&)> on_receive_complete_frame_ = nullptr;
  uint32_t last_complete_frame_ts_ = 0;
  RingBuffer<VideoFrame> compelete_video_frame_queue_;

 private:
  std::unique_ptr<RtpStatistics> rtp_statistics_ = nullptr;
  uint32_t last_send_rtcp_rr_packet_ts_ = 0;
  std::function<int(const char*, size_t)> data_send_func_ = nullptr;
};

#endif
