#include "ws_transmission.h"

#include "log.h"

WsTransmission::WsTransmission(
    std::function<void(const std::string &)> on_receive_msg_cb,
    std::function<void(WsStatus)> on_ws_status_cb)
    : on_receive_msg_(on_receive_msg_cb), on_ws_status_(on_ws_status_cb) {}

WsTransmission::~WsTransmission() {}

void WsTransmission::OnReceiveMessage(const std::string &msg) {
  // LOG_INFO("Receive msg: {}", msg);
  if (on_receive_msg_) {
    on_receive_msg_(msg);
  }
}

void WsTransmission::OnWsStatus(WsStatus ws_status) {
  // LOG_INFO("Receive msg: {}", msg);
  if (on_ws_status_) {
    on_ws_status_(ws_status);
  }
}