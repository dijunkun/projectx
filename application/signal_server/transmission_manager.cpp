#include "transmission_manager.h"

#include "log.h"

TransmissionManager::TransmissionManager() {}

TransmissionManager::~TransmissionManager() {}

std::vector<std::string> TransmissionManager::GetAllUserIdOfTransmission(
    const std::string& transmission_id) {
  if (transmission_user_id_list_.find(transmission_id) !=
      transmission_user_id_list_.end()) {
    return transmission_user_id_list_[transmission_id];
  }

  return std::vector<std::string>();
}

bool TransmissionManager::BindUserIdToTransmission(
    const std::string& user_id, const std::string& transmission_id) {
  if (transmission_user_id_list_.find(transmission_id) ==
      transmission_user_id_list_.end()) {
    transmission_user_id_list_[transmission_id].push_back(user_id);
    LOG_INFO("Bind user id [{}]  to transmission [{}]", user_id,
             transmission_id);
    return true;
  } else {
    auto user_id_list = transmission_user_id_list_[transmission_id];
    for (auto id : user_id_list) {
      if (id == user_id) {
        LOG_WARN("User id [{}] already bind to transmission [{}]", user_id,
                 transmission_id);
        return false;
      }
    }
    transmission_user_id_list_[transmission_id].push_back(user_id);
    LOG_INFO("Bind user id [{}]  to transmission [{}]", user_id,
             transmission_id);
  }
  return true;
}

bool TransmissionManager::BindUserIdToWsHandle(
    const std::string& user_id, websocketpp::connection_hdl hdl) {
  if (user_id_ws_hdl_list_.find(user_id) != user_id_ws_hdl_list_.end()) {
    LOG_WARN("User id already bind to websocket handle [{}]", user_id,
             hdl.lock().get());
    return false;
  } else {
    user_id_ws_hdl_list_[user_id] = hdl;
  }
  return true;
}

std::string TransmissionManager::ReleaseUserIdFromTransmission(
    websocketpp::connection_hdl hdl) {
  for (auto it = user_id_ws_hdl_list_.begin(); it != user_id_ws_hdl_list_.end();
       ++it) {
    if (it->second.lock().get() == hdl.lock().get()) {
      for (auto trans_it = transmission_user_id_list_.begin();
           trans_it != transmission_user_id_list_.end(); ++trans_it) {
        auto& user_id_list = trans_it->second;
        auto user_id_it =
            std::find(user_id_list.begin(), user_id_list.end(), it->first);
        if (user_id_it != user_id_list.end()) {
          user_id_list.erase(user_id_it);
          LOG_INFO("Remove user id [{}] from transmission [{}]", it->first,
                   trans_it->first);
          user_id_ws_hdl_list_.erase(it);
          return trans_it->first;
        }
      }
    }
  }
  return "";
}

bool TransmissionManager::ReleaseAllUserIdFromTransmission(
    const std::string& transmission_id) {
  if (transmission_user_id_list_.end() !=
      transmission_user_id_list_.find(transmission_id)) {
    auto user_id_list = transmission_user_id_list_[transmission_id];
    for (auto& user_id : user_id_list) {
      if (user_id_ws_hdl_list_.find(user_id) != user_id_ws_hdl_list_.end()) {
        LOG_INFO("Remove user id [{}] from transmission [{}]", user_id,
                 transmission_id);
        user_id_ws_hdl_list_.erase(user_id);
      }
    }

    user_id_list.clear();
    transmission_user_id_list_.erase(transmission_id);
  }
  return true;
}

websocketpp::connection_hdl TransmissionManager::GetWsHandle(
    const std::string& user_id) {
  if (user_id_ws_hdl_list_.find(user_id) != user_id_ws_hdl_list_.end()) {
    return user_id_ws_hdl_list_[user_id];
  } else {
    websocketpp::connection_hdl hdl;
    return hdl;
  }
}

std::string TransmissionManager::GetUserId(websocketpp::connection_hdl hdl) {
  for (auto it = user_id_ws_hdl_list_.begin(); it != user_id_ws_hdl_list_.end();
       ++it) {
    // LOG_INFO("[{}]", it->first);
    if (it->second.lock().get() == hdl.lock().get()) return it->first;
  }
  return "";
}