#ifndef _PEER_CONNECTION_H_
#define _PEER_CONNECTION_H_

#include <iostream>
#include <map>

#include "ice_transmission.h"
#include "ws_transmission.h"

enum SignalStatus { Connecting = 0, Connected, Closed };

typedef void (*OnReceiveBuffer)(unsigned char *, size_t, const char *,
                                const size_t);

typedef void (*NetStatusReport)(const unsigned short, const unsigned short);

typedef struct {
  const char *cfg_path;
  OnReceiveBuffer on_receive_buffer;
  NetStatusReport net_status_report;
} PeerConnectionParams;

class PeerConnection {
 public:
  PeerConnection();
  ~PeerConnection();

 public:
  int Create(PeerConnectionParams params, const std::string &id = "");
  int Join(PeerConnectionParams params, const std::string &id);
  int Destroy();

  SignalStatus GetSignalStatus();

  int SendData(const char *data, size_t size);

 private:
  std::string uri_ = "";
  std::string cfg_signal_server_ip_;
  std::string cfg_signal_server_port_;
  std::string cfg_stun_server_ip_;
  std::string cfg_stun_server_port_;
  int signal_server_port_ = 0;
  int stun_server_port_ = 0;
  WsTransmission *ws_transport_ = nullptr;
  IceTransmission *ice_transmission_ = nullptr;
  std::map<std::string, IceTransmission *> ice_transmission_list_;
  std::function<void(const std::string &)> on_receive_ws_msg_ = nullptr;
  std::function<void(const char *, size_t)> on_receive_ice_msg_ = nullptr;
  unsigned int ws_connection_id_ = 0;
  std::string transmission_id_ = "";
  SignalStatus signal_status_ = SignalStatus::Closed;
};

#endif