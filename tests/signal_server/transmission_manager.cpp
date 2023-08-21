#include "transmission_manager.h"

#include "log.h"

TransmissionManager::TransmissionManager() {}

TransmissionManager::~TransmissionManager() {}

// bool TransmissionManager::BindHostToTransmission(
//     websocketpp::connection_hdl hdl, const std::string& transmission_id) {
//   if (transmission_host_list_.find(transmission_id) !=
//       transmission_host_list_.end()) {
//     LOG_WARN("Transmission already has a host [{}]",
//              transmission_host_list_[transmission_id].lock().get());
//     return false;
//   } else {
//     transmission_host_list_[transmission_id] = hdl;
//   }
//   return true;
// }

// bool TransmissionManager::BindGuestToTransmission(
//     websocketpp::connection_hdl hdl, const std::string& transmission_id) {
//   if (transmission_guest_list_.find(transmission_id) !=
//       transmission_guest_list_.end()) {
//     transmission_guest_list_[transmission_id].push_back(hdl);
//   } else {
//     std::vector<websocketpp::connection_hdl> guest_hdl_list;
//     guest_hdl_list.push_back(hdl);
//     transmission_guest_list_[transmission_id] = guest_hdl_list;
//   }
//   return true;
// }

// bool TransmissionManager::ReleaseHostFromTransmission(
//     websocketpp::connection_hdl hdl, const std::string& transmission_id) {
//   return true;
// }

// bool TransmissionManager::ReleaseGuestFromTransmission(
//     websocketpp::connection_hdl hdl, const std::string& transmission_id) {
//   return true;
// }

// bool TransmissionManager::BindHostUsernameToWsHandle(
//     websocketpp::connection_hdl hdl) {
//   if (transmission_host_username_list_.find("host") !=
//       transmission_host_username_list_.end()) {
//     LOG_ERROR("Host already exist");
//     return false;
//   } else {
//     transmission_host_username_list_["host"] = hdl;
//   }
//   return true;
// }

// bool TransmissionManager::UpdateHostUsernameToWsHandle(
//     const std::string& host_username, websocketpp::connection_hdl hdl) {
//   if (transmission_host_username_list_.find("host") ==
//       transmission_host_username_list_.end()) {
//     LOG_ERROR("Host not exist");
//     return false;
//   }
//   transmission_host_username_list_.erase("host");
//   transmission_host_username_list_[host_username] = hdl;

//   return true;
// }

// bool TransmissionManager::BindGuestUsernameToWsHandle(
//     const std::string& guest_username, websocketpp::connection_hdl hdl) {
//   if (transmission_guest_username_list_.find(guest_username) !=
//       transmission_guest_username_list_.end()) {
//     LOG_ERROR("Guest already bind to username [{}]", guest_username.c_str());
//     return false;
//   } else {
//     transmission_guest_username_list_[guest_username] = hdl;
//   }
//   return true;
// }

// websocketpp::connection_hdl TransmissionManager::GetHostOfTransmission(
//     const std::string& transmission_id) {
//   if (transmission_host_list_.find(transmission_id) !=
//       transmission_host_list_.end()) {
//     return transmission_host_list_[transmission_id];
//   } else {
//     websocketpp::connection_hdl hdl;
//     return hdl;
//   }
// }

// std::string TransmissionManager::GetHostUsername(
//     websocketpp::connection_hdl hdl) {
//   for (auto host : transmission_host_username_list_) {
//     if (host.second.lock().get() == hdl.lock().get()) return host.first;
//   }

//   return "";
// }

// std::string TransmissionManager::GetGuestUsername(
//     websocketpp::connection_hdl hdl) {
//   for (auto guest : transmission_guest_username_list_) {
//     if (guest.second.lock().get() == hdl.lock().get()) return guest.first;
//   }

//   return "";
// }

std::vector<websocketpp::connection_hdl>
TransmissionManager::GetAllGuestsOfTransmission(
    const std::string& transmission_id) {
  if (transmission_user_ws_hdl_list_.find(transmission_id) !=
      transmission_user_ws_hdl_list_.end()) {
    return transmission_user_ws_hdl_list_[transmission_id];
  } else {
    return std::vector<websocketpp::connection_hdl>();
  }
}

websocketpp::connection_hdl TransmissionManager::GetGuestWsHandle(
    const std::string& guest_username) {
  if (transmission_guest_username_list_.find(guest_username) !=
      transmission_guest_username_list_.end()) {
    return transmission_guest_username_list_[guest_username];
  } else {
    websocketpp::connection_hdl hdl;
    return hdl;
  }
}

std::vector<std::string> TransmissionManager::GetAllMembersOfTransmission(
    const std::string& transmission_id) {
  std::vector<std::string> member_list;

  for (auto guest_hdl : GetAllGuestsOfTransmission(transmission_id)) {
    member_list.push_back(GetUsername(guest_hdl));
  }

  return member_list;
}

bool TransmissionManager::BindWsHandleToTransmission(
    websocketpp::connection_hdl hdl, const std::string& transmission_id) {
  if (transmission_user_ws_hdl_list_.find(transmission_id) ==
      transmission_user_ws_hdl_list_.end()) {
    transmission_user_ws_hdl_list_[transmission_id].push_back(hdl);
    return true;
  } else {
    auto hdl_list = transmission_user_ws_hdl_list_[transmission_id];
    for (auto h : hdl_list) {
      if (h.lock().get() == hdl.lock().get()) {
        LOG_ERROR("Ws handle [{}] already bind to transmission [{}]",
                  hdl.lock().get(), transmission_id);
        return false;
      }
    }
    transmission_user_ws_hdl_list_[transmission_id].push_back(hdl);
  }
  return true;
}

bool TransmissionManager::BindUserIdToTransmission(
    const std::string& user_id, const std::string& transmission_id) {
  if (transmission_user_id_list_.find(transmission_id) ==
      transmission_user_id_list_.end()) {
    transmission_user_id_list_[transmission_id].push_back(user_id);
    return true;
  } else {
    auto user_id_list = transmission_user_id_list_[transmission_id];
    for (auto id : user_id_list) {
      if (id == user_id) {
        LOG_ERROR("User id [{}] already bind to transmission [{}]", user_id,
                  transmission_id);
        return false;
      }
    }
    transmission_user_id_list_[transmission_id].push_back(user_id);
  }
  return true;
}

bool TransmissionManager::BindUserIdToWsHandle(
    const std::string& user_id, websocketpp::connection_hdl hdl) {
  if (user_id_ws_hdl_list_.find(user_id) != user_id_ws_hdl_list_.end()) {
    LOG_ERROR("User id already bind to websocket handle [{}]", user_id,
              hdl.lock().get());
    return false;
  } else {
    user_id_ws_hdl_list_[user_id] = hdl;
  }
  return true;
}

bool TransmissionManager::BindUserNameToUserId(const std::string& user_name,
                                               const std::string& user_id) {
  if (user_name_user_id_list_.find(user_id) == user_name_user_id_list_.end()) {
    user_name_user_id_list_[user_id].push_back(user_name);
    return true;
  } else {
    auto user_name_list = user_name_user_id_list_[user_id];
    for (auto name : user_name_list) {
      if (name == user_name) {
        LOG_ERROR("User name [{}] already bind to user id [{}]", user_name,
                  user_id);
        return false;
      }
    }
    user_name_user_id_list_[user_id].push_back(user_name);
  }
  return true;
}

bool TransmissionManager::ReleaseWsHandleFromTransmission(
    websocketpp::connection_hdl hdl, const std::string& transmission_id) {
  return true;
}

bool TransmissionManager::BindUsernameToWsHandle(
    const std::string& username, websocketpp::connection_hdl hdl) {
  if (username_ws_hdl_list_.find(username) != username_ws_hdl_list_.end()) {
    LOG_ERROR("Guest already bind to username [{}]", username.c_str());
    return false;
  } else {
    username_ws_hdl_list_[username] = hdl;
  }
  return true;
}

bool TransmissionManager::UpdateUsernameToWsHandle(
    const std::string& username, websocketpp::connection_hdl hdl) {
  if (username_ws_hdl_list_.find("host") == username_ws_hdl_list_.end()) {
    LOG_ERROR("Host not exist");
    return false;
  }
  username_ws_hdl_list_.erase("host");
  username_ws_hdl_list_[username] = hdl;

  return true;
}

std::string TransmissionManager::GetUsername(websocketpp::connection_hdl hdl) {
  for (auto guest : username_ws_hdl_list_) {
    if (guest.second.lock().get() == hdl.lock().get()) return guest.first;
  }

  LOG_ERROR("No user with websocket handle [{}]", hdl.lock().get());
  return "";
}

websocketpp::connection_hdl TransmissionManager::GetWsHandle(
    const std::string& username) {
  if (username_ws_hdl_list_.find(username) != username_ws_hdl_list_.end()) {
    return username_ws_hdl_list_[username];
  } else {
    websocketpp::connection_hdl hdl;
    return hdl;
  }
}