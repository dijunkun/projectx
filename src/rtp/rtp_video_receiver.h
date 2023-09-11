#ifndef _RTP_VIDEO_RECEIVER_H_
#define _RTP_VIDEO_RECEIVER_H_

#include <functional>
#include <map>
#include <mutex>
#include <queue>

#include "frame.h"
#include "ringbuffer.h"
#include "rtp_video_session.h"
#include "thread_base.h"

class RtpVideoReceiver : public ThreadBase {
 public:
  RtpVideoReceiver();
  ~RtpVideoReceiver();

 public:
  void InsertRtpPacket(RtpPacket& rtp_packet);

  void SetOnReceiveCompleteFrame(
      std::function<void(VideoFrame&)> on_receive_complete_frame) {
    on_receive_complete_frame_ = on_receive_complete_frame;
  }

  void Start();
  void Stop();

 private:
  bool CheckIsFrameCompleted(RtpPacket& rtp_packet);
  //   void OnReceiveFrame(uint8_t* payload) {}

 private:
  bool Process() override;

 private:
  std::map<uint16_t, RtpPacket> incomplete_frame_list_;
  uint8_t* nv12_data_ = nullptr;
  std::function<void(VideoFrame&)> on_receive_complete_frame_ = nullptr;
  uint32_t last_complete_frame_ts_ = 0;

  RingBuffer<VideoFrame> compelete_video_frame_queue_;

  bool stop_ = true;
  std::mutex mutex_;
};

#endif
