#ifndef _PEER_CONNECTION_H_
#define _PEER_CONNECTION_H_

#include <iostream>

#include "ice_transport.h"
#include "ws_transport.h"

enum SignalStatus { Connecting = 0, Connected, Closed };

class PeerConnection {
 public:
  PeerConnection();
  ~PeerConnection();

 public:
  int Init(std::string const &uri);
  int Init(std::string const &uri, std::string const &id);
  int Destroy();

  SignalStatus GetSignalStatus();

 private:
  WsTransport *ws_transport_ = nullptr;
  IceTransport *ice_transport_ = nullptr;
  std::function<void(const std::string &)> on_receive_ws_msg_ = nullptr;
  std::function<void(const char *, size_t)> on_receive_ice_msg_ = nullptr;
  unsigned int connection_id_ = 0;
  std::string transport_id_ = "";
  SignalStatus signal_status_ = SignalStatus::Closed;
};

#endif