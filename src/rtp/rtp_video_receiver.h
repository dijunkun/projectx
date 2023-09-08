#ifndef _RTP_VIDEO_RECEIVER_H_
#define _RTP_VIDEO_RECEIVER_H_

#include <functional>
#include <map>
#include <queue>

#include "frame.h"
#include "rtp_video_session.h"

class RtpVideoReceiver {
 public:
  RtpVideoReceiver();
  ~RtpVideoReceiver();

 public:
  void InsertRtpPacket(RtpPacket& rtp_packet);

  void SetOnReceiveCompleteFrame(
      std::function<void(VideoFrame&)> on_receive_complete_frame) {
    on_receive_complete_frame_ = on_receive_complete_frame;
  }

 private:
  bool CheckIsFrameCompleted(RtpPacket& rtp_packet);

  //  private:
  //   void OnReceiveFrame(uint8_t* payload) {}

 private:
  std::map<uint16_t, RtpPacket> incomplete_frame_list_;
  std::queue<VideoFrame> compelete_video_frame_queue_;
  uint8_t* nv12_data_ = nullptr;
  std::function<void(VideoFrame&)> on_receive_complete_frame_ = nullptr;
};

#endif
