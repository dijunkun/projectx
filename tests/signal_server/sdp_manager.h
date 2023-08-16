#ifndef _SDP_MANAGER_H_
#define _SDP_MANAGER_H_

#include <iostream>
#include <map>

class SdpManager {
 public:
  SdpManager();
  ~SdpManager();

 public:
  int AddOfferSdpToConnection(std::string &sdp, std::string &connection_id);
  int AddAnswerSdpToConnection(std::string &sdp, std::string &connection_id);
  int UpdateOfferSdpToConnection(std::string &sdp, std::string &connection_id);
  int UpdateAnswerSdpToConnection(std::string &sdp, std::string &connection_id);

  const std::string &GetOfferSdpFromConnection(std::string &connection_id);
  const std::string &GetAnswerSdpFromConnection(std::string &connection_id);

  int RemoveConnetion(std::string &connection_id);

 private:
  // <connection_id, <offer_sdp, answer_sdp>.
  std::map<std::string, std::pair<std::string, std::string>> answer_hdl_map_;
};

#endif