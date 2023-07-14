#include "ws_core.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

#include "log.h"

WsCore::WsCore() {
  m_endpoint_.clear_access_channels(websocketpp::log::alevel::all);
  m_endpoint_.clear_error_channels(websocketpp::log::elevel::all);

  m_endpoint_.init_asio();
  m_endpoint_.start_perpetual();

  m_thread_ = websocketpp::lib::make_shared<websocketpp::lib::thread>(
      &client::run, &m_endpoint_);
}

WsCore::~WsCore() {
  m_endpoint_.stop_perpetual();

  if (GetStatus() != "Open") {
    // Only close open connections
    return;
  }

  websocketpp::lib::error_code ec;
  m_endpoint_.close(connection_handle_, websocketpp::close::status::going_away,
                    "", ec);
  if (ec) {
    LOG_INFO("> Error closing connection {}", ec.message());
  }

  m_thread_->join();
}

int WsCore::Connect(std::string const &uri) {
  websocketpp::lib::error_code ec;

  client::connection_ptr con = m_endpoint_.get_connection(uri, ec);

  connection_handle_ = con->get_handle();

  if (ec) {
    LOG_INFO("> Connect initialization error: {}", ec.message());
    return -1;
  }

  con->set_open_handler(websocketpp::lib::bind(
      &WsCore::OnOpen, this, &m_endpoint_, websocketpp::lib::placeholders::_1));
  con->set_fail_handler(websocketpp::lib::bind(
      &WsCore::OnFail, this, &m_endpoint_, websocketpp::lib::placeholders::_1));
  con->set_close_handler(
      websocketpp::lib::bind(&WsCore::OnClose, this, &m_endpoint_,
                             websocketpp::lib::placeholders::_1));

  // con->set_ping_handler(websocketpp::lib::bind(
  //     &WsCore::on_ping,
  //     this,
  //     websocketpp::lib::placeholders::_1,
  //     websocketpp::lib::placeholders::_2
  // ));

  con->set_pong_handler(websocketpp::lib::bind(
      &WsCore::OnPong, this, websocketpp::lib::placeholders::_1,
      websocketpp::lib::placeholders::_2));

  con->set_pong_timeout(1000);

  con->set_pong_timeout_handler(websocketpp::lib::bind(
      &WsCore::OnPongTimeout, this, websocketpp::lib::placeholders::_1,
      websocketpp::lib::placeholders::_2));

  con->set_message_handler(websocketpp::lib::bind(
      &WsCore::OnMessage, this, websocketpp::lib::placeholders::_1,
      websocketpp::lib::placeholders::_2));

  m_endpoint_.connect(con);

  return 0;
}

void WsCore::Close(websocketpp::close::status::value code, std::string reason) {
  websocketpp::lib::error_code ec;

  m_endpoint_.close(connection_handle_, code, reason, ec);
  if (ec) {
    LOG_INFO("> Error initiating close: {}", ec.message());
  }
}

void WsCore::Send(std::string message) {
  websocketpp::lib::error_code ec;

  m_endpoint_.send(connection_handle_, message,
                   websocketpp::frame::opcode::text, ec);
  if (ec) {
    LOG_INFO("> Error sending message: {}", ec.message());
    return;
  }
}

void WsCore::Ping() {
  websocketpp::lib::error_code ec;

  std::string message = "ping";

  m_endpoint_.ping(connection_handle_, message, ec);
  if (ec) {
    LOG_INFO("> Error sending ping");
    return;
  }
}

const std::string &WsCore::GetStatus() { return connection_status_; }

void WsCore::OnOpen(client *c, websocketpp::connection_hdl hdl) {
  connection_status_ = "Open";
}

void WsCore::OnFail(client *c, websocketpp::connection_hdl hdl) {
  connection_status_ = "Failed";
}

void WsCore::OnClose(client *c, websocketpp::connection_hdl hdl) {
  connection_status_ = "Closed";
}

void WsCore::OnPong(websocketpp::connection_hdl, std::string msg) {}

void WsCore::OnPongTimeout(websocketpp::connection_hdl, std::string msg) {}

void WsCore::OnMessage(websocketpp::connection_hdl, client::message_ptr msg) {
  OnReceiveMessage(msg->get_payload());
}