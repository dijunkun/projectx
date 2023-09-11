#ifndef _RTP_VIDEO_RECEIVE_STATISTICS_H_
#define _RTP_VIDEO_RECEIVE_STATISTICS_H_

#include "thread_base.h"

class RtpVideoReceiveStatistics : public ThreadBase {
 public:
  RtpVideoReceiveStatistics();
  ~RtpVideoReceiveStatistics();

 public:
  void UpdateReceiveBytes(uint32_t received_bytes);

 private:
  bool Process();

 private:
  uint32_t received_bytes_ = 0;
};

#endif