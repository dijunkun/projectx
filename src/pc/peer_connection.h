#ifndef _PEER_CONNECTION_H_
#define _PEER_CONNECTION_H_

#include <iostream>

#include "ice_transport.h"
#include "ws_transport.h"

enum SignalStatus { Connecting = 0, Connected, Closed };

class PeerConnection {
 public:
  typedef void (*OnReceiveBuffer)(unsigned char *, size_t, const char *,
                                  const size_t);

  typedef void (*NetStatusReport)(const unsigned short, const unsigned short);

  typedef struct {
    const char *cfg_path;
    OnReceiveBuffer on_receive_buffer;
    NetStatusReport net_status_report;
  } Params;

 public:
  PeerConnection();
  ~PeerConnection();

 public:
  int Init(Params params);
  int Init(Params params, std::string const &id);
  int Destroy();

  SignalStatus GetSignalStatus();

  int SendData(const char *data, size_t size);

 private:
  std::string uri_ = "";
  WsTransport *ws_transport_ = nullptr;
  IceTransport *ice_transport_ = nullptr;
  std::function<void(const std::string &)> on_receive_ws_msg_ = nullptr;
  std::function<void(const char *, size_t)> on_receive_ice_msg_ = nullptr;
  unsigned int connection_id_ = 0;
  std::string transport_id_ = "";
  SignalStatus signal_status_ = SignalStatus::Closed;
};

#endif