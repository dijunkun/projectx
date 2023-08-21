#ifndef _WS_TRANSMISSION_H_
#define _WS_TRANSMISSION_H_

#include "ws_core.h"

class WsTransmission : public WsCore {
 public:
  WsTransmission(std::function<void(const std::string &)> on_receive_msg_cb);
  ~WsTransmission();

 public:
  void OnReceiveMessage(const std::string &msg);

 private:
  std::function<void(const std::string &)> on_receive_msg_ = nullptr;
};

#endif