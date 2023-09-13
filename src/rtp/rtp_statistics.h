#ifndef _RTP_STATISTICS_H_
#define _RTP_STATISTICS_H_

#include "thread_base.h"

class RtpStatistics : public ThreadBase {
 public:
  RtpStatistics();
  ~RtpStatistics();

 public:
  void UpdateSentBytes(uint32_t sent_bytes);
  void UpdateReceiveBytes(uint32_t received_bytes);

 private:
  bool Process();

 private:
  uint32_t sent_bytes_ = 0;
  uint32_t received_bytes_ = 0;
};

#endif