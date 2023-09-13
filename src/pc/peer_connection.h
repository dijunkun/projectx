#ifndef _PEER_CONNECTION_H_
#define _PEER_CONNECTION_H_

#include <iostream>
#include <map>

#include "ice_transmission.h"
#include "nv_decoder.h"
#include "nv_encoder.h"
#include "ws_transmission.h"

enum SignalStatus { Connecting = 0, Connected, Closed };

typedef void (*OnReceiveBuffer)(const char *, size_t, const char *,
                                const size_t);

typedef void (*NetStatusReport)(const unsigned short, const unsigned short);

typedef struct {
  const char *cfg_path;
  OnReceiveBuffer on_receive_buffer;
  NetStatusReport net_status_report;
} PeerConnectionParams;

class PeerConnection : public VideoEncoder, VideoDecoder {
 public:
  PeerConnection(OnReceiveBuffer on_receive_buffer);
  ~PeerConnection();

 public:
  int Create(PeerConnectionParams params,
             const std::string &transmission_id = "",
             const std::string &user_id = "");

  int Join(PeerConnectionParams params, const std::string &transmission_id,
           const std::string &user_id = "");

  int Destroy();

  SignalStatus GetSignalStatus();

  int SendVideoData(const char *data, size_t size);
  int SendAudioData(const char *data, size_t size);
  int SendUserData(const char *data, size_t size);

 private:
  int Init(PeerConnectionParams params, const std::string &transmission_id,
           const std::string &user_id);

  void ProcessSignal(const std::string &signal);

  int RequestTransmissionMemberList(const std::string &transmission_id);

 private:
  std::string uri_ = "";
  std::string cfg_signal_server_ip_;
  std::string cfg_signal_server_port_;
  std::string cfg_stun_server_ip_;
  std::string cfg_stun_server_port_;
  int signal_server_port_ = 0;
  int stun_server_port_ = 0;

 private:
  std::shared_ptr<WsTransmission> ws_transport_ = nullptr;
  std::function<void(const std::string &)> on_receive_ws_msg_ = nullptr;
  unsigned int ws_connection_id_ = 0;
  std::string user_id_ = "";
  std::string transmission_id_ = "";
  std::vector<std::string> user_id_list_;
  SignalStatus signal_status_ = SignalStatus::Closed;

 private:
  std::map<std::string, std::unique_ptr<IceTransmission>>
      ice_transmission_list_;
  std::function<void(const char *, size_t, const char *, size_t)>
      on_receive_ice_msg_ = nullptr;
  std::function<void(std::string)> on_ice_status_change_ = nullptr;
  bool ice_ready_ = false;

  OnReceiveBuffer on_receive_buffer_;
  char *nv12_data_ = nullptr;
};

#endif