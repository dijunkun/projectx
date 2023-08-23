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
  //   bool BindHostToTransmission(websocketpp::connection_hdl hdl,
  //                               const std::string& transmission_id);
  //   bool BindGuestToTransmission(websocketpp::connection_hdl hdl,
  //                                const std::string& transmission_id);
  //   bool ReleaseHostFromTransmission(websocketpp::connection_hdl hdl,
  //                                    const std::string& transmission_id);
  //   bool ReleaseGuestFromTransmission(websocketpp::connection_hdl hdl,
  //                                     const std::string& transmission_id);

  //   bool BindHostUsernameToWsHandle(websocketpp::connection_hdl hdl);
  //   bool UpdateHostUsernameToWsHandle(const std::string& host_username,
  //                                     websocketpp::connection_hdl hdl);
  //   bool BindGuestUsernameToWsHandle(const std::string& guest_username,
  //                                    websocketpp::connection_hdl hdl);

  //   std::string GetHostUsername(websocketpp::connection_hdl hdl);
  //   std::string GetGuestUsername(websocketpp::connection_hdl hdl);

  websocketpp::connection_hdl GetHostOfTransmission(
      const std::string& transmission_id);
  std::vector<websocketpp::connection_hdl> GetAllGuestsOfTransmission(
      const std::string& transmission_id);
  websocketpp::connection_hdl GetGuestWsHandle(
      const std::string& guest_username);

  std::vector<std::string> GetAllMembersOfTransmission(
      const std::string& transmission_id);

 public:
  bool BindWsHandleToTransmission(websocketpp::connection_hdl hdl,
                                  const std::string& transmission_id);
  bool BindUserIdToTransmission(const std::string& user_id,
                                const std::string& transmission_id);
  bool BindUserIdToWsHandle(const std::string& user_id,
                            websocketpp::connection_hdl hdl);
  bool BindUserNameToUserId(const std::string& user_name,
                            const std::string& user_id);

  bool ReleaseWsHandleFromTransmission(websocketpp::connection_hdl hdl,
                                       const std::string& transmission_id);
  bool BindUsernameToWsHandle(const std::string& username,
                              websocketpp::connection_hdl hdl);
  bool UpdateUsernameToWsHandle(const std::string& username,
                                websocketpp::connection_hdl hdl);
  std::string GetUsername(websocketpp::connection_hdl hdl);
  //   websocketpp::connection_hdl GetWsHandle(const std::string& username);
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
  std::map<std::string, std::vector<std::string>> transmission_user_id_list_;
  std::map<std::string, websocketpp::connection_hdl> user_id_ws_hdl_list_;
  std::map<std::string, std::vector<std::string>> user_name_user_id_list_;

  std::map<std::string, websocketpp::connection_hdl> username_ws_hdl_list_;
};

#endif