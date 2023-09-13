#ifndef _RTP_TRANSCEIVER_H_
#define _RTP_TRANSCEIVER_H_

#include <functional>

class RtpTransceiver {
 public:
  RtpTransceiver();
  ~RtpTransceiver();

 public:
  virtual void SetSendDataFunc(
      std::function<int(const char *, size_t)> data_send_func) = 0;

  virtual void OnReceiveData(const char *data, size_t size) = 0;
};

#endif