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
  bool BindHostToTransmission(websocketpp::connection_hdl hdl,
                              const std::string& transmission_id);
  bool BindGuestToTransmission(websocketpp::connection_hdl hdl,
                               const std::string& transmission_id);
  bool ReleaseHostFromTransmission(websocketpp::connection_hdl hdl,
                                   const std::string& transmission_id);
  bool ReleaseGuestFromTransmission(websocketpp::connection_hdl hdl,
                                    const std::string& transmission_id);

  bool BindHostUsernameToWsHandle(const std::string& host_username,
                                  websocketpp::connection_hdl hdl);
  bool BindGuestUsernameToWsHandle(const std::string& guest_username,
                                   websocketpp::connection_hdl hdl);

  std::string GetHostUsername(websocketpp::connection_hdl hdl);
  std::string GetGuestUsername(websocketpp::connection_hdl hdl);

  websocketpp::connection_hdl GetHostOfTransmission(
      const std::string& transmission_id);
  std::vector<websocketpp::connection_hdl> GetAllGuestsOfTransmission(
      const std::string& transmission_id);
  websocketpp::connection_hdl GetGuestWsHandle(
      const std::string& guest_username);

  std::vector<std::string> GetAllMembersOfTransmission(
      const std::string& transmission_id);

 private:
  std::map<std::string, websocketpp::connection_hdl> transmission_host_list_;
  std::map<std::string, std::vector<websocketpp::connection_hdl>>
      transmission_guest_list_;

  std::map<std::string, websocketpp::connection_hdl>
      transmission_host_username_list_;
  std::map<std::string, websocketpp::connection_hdl>
      transmission_guest_username_list_;
};

#endif