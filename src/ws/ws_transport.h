#ifndef _WS_TRANSPORT_H_
#define _WS_TRANSPORT_H_

#include "ws_core.h"

class WsTransport : public WsCore {
 public:
  WsTransport(std::function<void(const std::string &)> on_receive_msg_cb);
  ~WsTransport();

 public:
  void OnReceiveMessage(const std::string &msg);

 private:
  std::function<void(const std::string &)> on_receive_msg_ = nullptr;
};

#endif