#ifndef _ICE_TRANSPORT_H_
#define _ICE_TRANSPORT_H_

#include <iostream>

#include "ice_agent.h"
#include "ws_transport.h"

class IceTransport {
 public:
  IceTransport(WsTransport *ice_ws_transport,
               std::function<void(const char *, size_t)> on_receive_ice_msg);

  ~IceTransport();

  int InitIceTransport(std::string &ip, int port);
  int InitIceTransport(std::string &ip, int port, std::string const &id);

  int DestroyIceTransport();

  int CreateTransport();
  int CreateTransport(std::string transport_id);

  int SendData(const char *data, size_t size);

  void OnReceiveUserData(const char *data, size_t size);

  void OnReceiveMessage(const std::string &msg);

 private:
  int GatherCandidates();

  int GetLocalSdp();

  int QueryRemoteSdp(std::string transport_id);

  int SetRemoteSdp(const std::string &remote_sdp);

  int AddRemoteCandidate(const std::string &remote_candidate);

  int CreateOffer();

  int SendOffer();

  int CreateAnswer();

  int SendAnswer();

  int SendOfferLocalCandidate(const std::string &remote_candidate);

  int SendAnswerLocalCandidate(const std::string &remote_candidate);

 private:
  IceAgent *ice_agent_ = nullptr;
  WsTransport *ice_ws_transport_ = nullptr;
  std::function<void(const char *, size_t)> on_receive_ice_msg_cb_ = nullptr;
  std::string local_sdp_;
  std::string remote_sdp_;
  std::string local_candidates_;
  std::string remote_candidates_;
  unsigned int connection_id_ = 0;
  std::string transport_id_ = "";
  bool offer_peer_ = true;
};

#endif