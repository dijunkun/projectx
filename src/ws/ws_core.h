#ifndef _WS_CORE_H_
#define _WS_CORE_H_

#include <map>
#include <sstream>
#include <string>

#include "websocketpp/client.hpp"
#include "websocketpp/common/memory.hpp"
#include "websocketpp/common/thread.hpp"
#include "websocketpp/config/asio_no_tls_client.hpp"

typedef websocketpp::client<websocketpp::config::asio_client> client;

class WsCore {
 public:
  WsCore();

  virtual ~WsCore();

  int Connect(std::string const &uri);

  void Close(websocketpp::close::status::value code, std::string reason);

  void Send(std::string message);

  void Ping();

  const std::string &GetStatus();

  // Callback
  void OnOpen(client *c, websocketpp::connection_hdl hdl);

  void OnFail(client *c, websocketpp::connection_hdl hdl);

  void OnClose(client *c, websocketpp::connection_hdl hdl);

  void OnPong(websocketpp::connection_hdl, std::string msg);

  void OnPongTimeout(websocketpp::connection_hdl, std::string msg);

  void OnMessage(websocketpp::connection_hdl, client::message_ptr msg);

  virtual void OnReceiveMessage(const std::string &msg) = 0;

 private:
  client m_endpoint_;
  websocketpp::connection_hdl connection_handle_;
  websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread_;

  std::string connection_status_ = "Connecting";
};

#endif