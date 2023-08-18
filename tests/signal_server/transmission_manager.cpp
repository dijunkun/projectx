#include "transmission_manager.h"

#include "log.h"

TransmissionManager::TransmissionManager() {}

TransmissionManager::~TransmissionManager() {}

bool TransmissionManager::BindHostToTransmission(
    websocketpp::connection_hdl hdl, const std::string& transmission_id) {
  if (transmission_host_list_.find(transmission_id) !=
      transmission_host_list_.end()) {
    LOG_WARN("Transmission already has a host [{}]",
             transmission_host_list_[transmission_id].lock().get());
    return false;
  } else {
    transmission_host_list_[transmission_id] = hdl;
  }
  return true;
}

bool TransmissionManager::BindGuestToTransmission(
    websocketpp::connection_hdl hdl, const std::string& transmission_id) {
  if (transmission_guest_list_.find(transmission_id) !=
      transmission_guest_list_.end()) {
    transmission_guest_list_[transmission_id].push_back(hdl);
  } else {
    std::vector<websocketpp::connection_hdl> guest_hdl_list;
    guest_hdl_list.push_back(hdl);
    transmission_guest_list_[transmission_id] = guest_hdl_list;
  }
  return true;
}

bool TransmissionManager::ReleaseHostFromTransmission(
    websocketpp::connection_hdl hdl, const std::string& transmission_id) {
  return true;
}

bool TransmissionManager::ReleaseGuestFromTransmission(
    websocketpp::connection_hdl hdl, const std::string& transmission_id) {
  return true;
}

bool TransmissionManager::BindHostUsernameToWsHandle(
    const std::string& host_username, websocketpp::connection_hdl hdl) {
  if (transmission_host_username_list_.find(host_username) !=
      transmission_host_username_list_.end()) {
    LOG_ERROR("Guest already bind to username [{}]", host_username.c_str());
    return false;
  } else {
    transmission_host_username_list_[host_username] = hdl;
  }
  return true;
}

bool TransmissionManager::BindGuestUsernameToWsHandle(
    const std::string& guest_username, websocketpp::connection_hdl hdl) {
  if (transmission_guest_username_list_.find(guest_username) !=
      transmission_guest_username_list_.end()) {
    LOG_ERROR("Guest already bind to username [{}]", guest_username.c_str());
    return false;
  } else {
    transmission_guest_username_list_[guest_username] = hdl;
  }
  return true;
}

websocketpp::connection_hdl TransmissionManager::GetHostOfTransmission(
    const std::string& transmission_id) {
  if (transmission_host_list_.find(transmission_id) !=
      transmission_host_list_.end()) {
    return transmission_host_list_[transmission_id];
  } else {
    websocketpp::connection_hdl hdl;
    return hdl;
  }
}

std::string TransmissionManager::GetHostUsername(
    websocketpp::connection_hdl hdl) {
  for (auto host : transmission_host_username_list_) {
    if (host.second.lock().get() == hdl.lock().get()) return host.first;
  }

  return "";
}

std::string TransmissionManager::GetGuestUsername(
    websocketpp::connection_hdl hdl) {
  for (auto guest : transmission_guest_username_list_) {
    if (guest.second.lock().get() == hdl.lock().get()) return guest.first;
  }

  return "";
}

std::vector<websocketpp::connection_hdl>
TransmissionManager::GetAllGuestsOfTransmission(
    const std::string& transmission_id) {
  if (transmission_guest_list_.find(transmission_id) !=
      transmission_guest_list_.end()) {
    return transmission_guest_list_[transmission_id];
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

  member_list.push_back(
      GetHostUsername(GetHostOfTransmission(transmission_id)));

  for (auto guest_hdl : GetAllGuestsOfTransmission(transmission_id)) {
    member_list.push_back(GetGuestUsername(guest_hdl));
  }

  return member_list;
}