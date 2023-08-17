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

bool TransmissionManager::BindGuestUsernameToWsHandle(
    const std::string& guest_username, websocketpp::connection_hdl hdl) {
  if (transmission_guest_username_list_.find(guest_username) !=
      transmission_guest_username_list_.end()) {
    LOG_WARN("Guest already bind to username [{}]", guest_username.c_str());
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

std::set<websocketpp::connection_hdl>
TransmissionManager::GetGuestOfTransmission(
    const std::string& transmission_id) {
  if (transmission_guest_list_.find(transmission_id) !=
      transmission_guest_list_.end()) {
    return transmission_guest_list_[transmission_id];
  } else {
    return std::set<websocketpp::connection_hdl>();
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