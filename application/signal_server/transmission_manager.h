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
  bool BindPasswordToTransmission(const std::string& password,
                                  const std::string& transmission_id);
  bool BindUserIdToWsHandle(const std::string& user_id,
                            websocketpp::connection_hdl hdl);

  std::string ReleaseUserIdFromTransmission(websocketpp::connection_hdl hdl);
  bool ReleaseAllUserIdFromTransmission(const std::string& transmission_id);
  bool ReleasePasswordFromTransmission(const std::string& transmission_id);

  websocketpp::connection_hdl GetWsHandle(const std::string& user_id);
  std::string GetUserId(websocketpp::connection_hdl hdl);
  bool CheckPassword(const std::string& password,
                     const std::string& transmission_id);
  std::string GetPassword(const std::string& transmission_id);

 private:
  std::map<std::string, std::vector<std::string>> transmission_user_id_list_;
  std::map<std::string, std::string> transmission_password_list_;
  std::map<std::string, websocketpp::connection_hdl> user_id_ws_hdl_list_;
};

#endif