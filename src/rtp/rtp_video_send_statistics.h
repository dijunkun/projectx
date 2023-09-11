#ifndef _RTP_VIDEO_SEND_STATISTICS_H_
#define _RTP_VIDEO_SEND_STATISTICS_H_

#include "thread_base.h"

class RtpVideoSendStatistics : public ThreadBase {
 public:
  RtpVideoSendStatistics();
  ~RtpVideoSendStatistics();

 public:
  void UpdateSentBytes(uint32_t sent_bytes);

 private:
  bool Process();

 private:
  uint32_t sent_bytes_ = 0;
};

#endif