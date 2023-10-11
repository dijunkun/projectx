#ifndef _PEER_CONNECTION_H_
#define _PEER_CONNECTION_H_

#include <iostream>
#include <map>
#include <mutex>

#include "ice_transmission.h"
#include "video_decoder_factory.h"
#include "video_encoder_factory.h"
#include "ws_transmission.h"
#include "x.h"

enum SignalStatus { SignalConnecting = 0, SignalConnected, SignalClosed };

typedef void (*OnReceiveBuffer)(const char *, size_t, const char *,
                                const size_t);

typedef void (*OnConnectionStatus)(ConnectionStatus status);

typedef void (*NetStatusReport)(const unsigned short, const unsigned short);

typedef struct {
  const char *cfg_path;
  OnReceiveBuffer on_receive_video_buffer;
  OnReceiveBuffer on_receive_audio_buffer;
  OnReceiveBuffer on_receive_data_buffer;
  OnConnectionStatus on_connection_status;
  NetStatusReport net_status_report;
} PeerConnectionParams;

class PeerConnection {
 public:
  PeerConnection();
  ~PeerConnection();

 public:
  int Init(PeerConnectionParams params, const std::string &user_id);

  int Create(PeerConnectionParams params,
             const std::string &transmission_id = "",
             const std::string &password = "");

  int Join(PeerConnectionParams params, const std::string &transmission_id,
           const std::string &password = "");

  int Leave();

  int Destroy();

  SignalStatus GetSignalStatus();

  int SendVideoData(const char *data, size_t size);
  int SendAudioData(const char *data, size_t size);
  int SendUserData(const char *data, size_t size);

 private:
  int CreateVideoCodec(bool hardware_acceleration);

  void ProcessSignal(const std::string &signal);

  int RequestTransmissionMemberList(const std::string &transmission_id,
                                    const std::string &password);

 private:
  std::string uri_ = "";
  std::string cfg_signal_server_ip_;
  std::string cfg_signal_server_port_;
  std::string cfg_stun_server_ip_;
  std::string cfg_stun_server_port_;
  std::string cfg_turn_server_ip_;
  std::string cfg_turn_server_port_;
  std::string cfg_turn_server_username_;
  std::string cfg_turn_server_password_;
  std::string cfg_hardware_acceleration_;
  int signal_server_port_ = 0;
  int stun_server_port_ = 0;
  int turn_server_port_ = 0;
  bool hardware_acceleration_ = false;

 private:
  std::shared_ptr<WsTransmission> ws_transport_ = nullptr;
  std::function<void(const std::string &)> on_receive_ws_msg_ = nullptr;
  unsigned int ws_connection_id_ = 0;
  std::string user_id_ = "";
  std::string transmission_id_ = "";
  std::vector<std::string> user_id_list_;
  SignalStatus signal_status_ = SignalStatus::SignalClosed;
  std::mutex signal_status_mutex_;

 private:
  std::map<std::string, std::unique_ptr<IceTransmission>>
      ice_transmission_list_;
  std::function<void(const char *, size_t, const char *, size_t)>
      on_receive_video_ = nullptr;
  std::function<void(const char *, size_t, const char *, size_t)>
      on_receive_audio_ = nullptr;
  std::function<void(const char *, size_t, const char *, size_t)>
      on_receive_data_ = nullptr;
  std::function<void(std::string)> on_ice_status_change_ = nullptr;
  bool ice_ready_ = false;

  OnReceiveBuffer on_receive_video_buffer_;
  OnReceiveBuffer on_receive_audio_buffer_;
  OnReceiveBuffer on_receive_data_buffer_;
  OnConnectionStatus on_connection_status_;
  char *nv12_data_ = nullptr;
  bool inited_ = false;
  std::string password_;

 private:
  std::unique_ptr<VideoEncoder> video_encoder_ = nullptr;
  std::unique_ptr<VideoDecoder> video_decoder_ = nullptr;
  bool hardware_accelerated_encode_ = false;
  bool hardware_accelerated_decode_ = false;
  bool b_force_i_frame_ = false;
};

#endif