#include "ws_transport.h"

#include "log.h"

WsTransport::WsTransport(
    std::function<void(const std::string &)> on_receive_msg_cb)
    : on_receive_msg_(on_receive_msg_cb) {}

WsTransport::~WsTransport() {}

void WsTransport::OnReceiveMessage(const std::string &msg) {
  LOG_INFO("Receive msg: {}", msg);
  if (on_receive_msg_) {
    on_receive_msg_(msg);
  }
}