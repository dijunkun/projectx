#ifndef _WS_TRANSMISSION_H_
#define _WS_TRANSMISSION_H_

#include "ws_core.h"

class WsTransmission : public WsCore {
 public:
  WsTransmission(std::function<void(const std::string &)> on_receive_msg_cb,
                 std::function<void(WsStatus)> on_ws_status_cb);
  ~WsTransmission();

 public:
  void OnReceiveMessage(const std::string &msg);

  void OnWsStatus(WsStatus ws_status);

 private:
  std::function<void(const std::string &)> on_receive_msg_ = nullptr;
  std::function<void(WsStatus)> on_ws_status_ = nullptr;
};

#endif