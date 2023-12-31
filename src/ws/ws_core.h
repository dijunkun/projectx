#ifndef _WS_CORE_H_
#define _WS_CORE_H_

#include <map>
#include <sstream>
#include <string>
#include <thread>

#include "websocketpp/client.hpp"
#include "websocketpp/common/memory.hpp"
#include "websocketpp/common/thread.hpp"
#include "websocketpp/config/asio_no_tls_client.hpp"

typedef websocketpp::client<websocketpp::config::asio_client> client;

enum WsStatus { WsOpening = 0, WsOpened, WsFailed, WsClosed, WsReconnecting };

class WsCore {
 public:
  WsCore();

  virtual ~WsCore();

  int Connect(std::string const &uri);

  void Close(websocketpp::close::status::value code, std::string reason);

  void Send(std::string message);

  void Ping(websocketpp::connection_hdl hdl);

  WsStatus GetStatus();

  // Callback
  void OnOpen(client *c, websocketpp::connection_hdl hdl);

  void OnFail(client *c, websocketpp::connection_hdl hdl);

  void OnClose(client *c, websocketpp::connection_hdl hdl);

  bool OnPing(websocketpp::connection_hdl hdl, std::string msg);

  bool OnPong(websocketpp::connection_hdl hdl, std::string msg);

  void OnPongTimeout(websocketpp::connection_hdl hdl, std::string msg);

  void OnMessage(websocketpp::connection_hdl hdl, client::message_ptr msg);

  virtual void OnReceiveMessage(const std::string &msg) = 0;

  virtual void OnWsStatus(WsStatus ws_status) = 0;

 private:
  client m_endpoint_;
  websocketpp::connection_hdl connection_handle_;
  websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread_;
  websocketpp::lib::shared_ptr<websocketpp::lib::thread> ping_thread_;

  WsStatus ws_status_ = WsStatus::WsClosed;
  int timeout_count_ = 0;
  std::string uri_;
};

#endif