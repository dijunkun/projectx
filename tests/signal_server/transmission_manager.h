#ifndef _TRANSIMISSION_MANAGER_H_
#define _TRANSIMISSION_MANAGER_H_

#include <map>
#include <set>
#include <websocketpp/server.hpp>

class TransmissionManager {
 public:
  TransmissionManager();
  ~TransmissionManager();

 public:
  std::vector<std::string> GetAllUserIdOfTransmission(
      const std::string& transmission_id);

 public:
  bool BindUserIdToTransmission(const std::string& user_id,
                                const std::string& transmission_id);
  bool BindUserIdToWsHandle(const std::string& user_id,
                            websocketpp::connection_hdl hdl);

  bool ReleaseWsHandleFromTransmission(websocketpp::connection_hdl hdl);
  bool ReleaseUserIdFromTransmission(websocketpp::connection_hdl hdl);

  websocketpp::connection_hdl GetWsHandle(const std::string& user_id);

 private:
  std::map<std::string, websocketpp::connection_hdl> transmission_host_list_;
  std::map<std::string, std::vector<websocketpp::connection_hdl>>
      transmission_guest_list_;

  std::map<std::string, websocketpp::connection_hdl>
      transmission_host_username_list_;
  std::map<std::string, websocketpp::connection_hdl>
      transmission_guest_username_list_;

 private:
  std::map<std::string, std::vector<websocketpp::connection_hdl>>
      transmission_user_ws_hdl_list_;
  std::map<std::string, websocketpp::connection_hdl> username_ws_hdl_list_;

  std::map<std::string, std::vector<std::string>> user_name_user_id_list_;

  std::map<std::string, std::vector<std::string>> transmission_user_id_list_;
  std::map<std::string, websocketpp::connection_hdl> user_id_ws_hdl_list_;
};

#endif