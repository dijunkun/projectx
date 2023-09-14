#ifndef _RTP_DATA_RECEIVER_H_
#define _RTP_DATA_RECEIVER_H_

#include <functional>

#include "rtcp_receiver_report.h"
#include "rtp_codec.h"
#include "rtp_statistics.h"

class RtpDataReceiver {
 public:
  RtpDataReceiver();
  ~RtpDataReceiver();

 public:
  void InsertRtpPacket(RtpPacket& rtp_packet);

  void SetSendDataFunc(std::function<int(const char*, size_t)> data_send_func);

  void SetOnReceiveData(
      std::function<void(const char*, size_t)> on_receive_data) {
    on_receive_data_ = on_receive_data;
  }

 private:
  bool CheckIsTimeSendRR();
  int SendRtcpRR(RtcpReceiverReport& rtcp_rr);

 private:
  std::function<void(const char*, size_t)> on_receive_data_ = nullptr;
  uint32_t last_complete_frame_ts_ = 0;

 private:
  std::unique_ptr<RtpStatistics> rtp_statistics_ = nullptr;
  uint32_t last_send_rtcp_rr_packet_ts_ = 0;
  std::function<int(const char*, size_t)> data_send_func_ = nullptr;
};

#endif