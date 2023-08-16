#ifndef _WS_HANDLE_MANAGER_H_
#define _WS_HANDLE_MANAGER_H_

#include <iostream>
#include <websocketpp/server.hpp>

class WsHandleManager {
 public:
  WsHandleManager();
  ~WsHandleManager();

 public:
  bool BindHandleToConnection(websocketpp::connection_hdl hdl,
                              std::string& connection_id);
  bool ReleaseHandleFromConnection(websocketpp::connection_hdl hdl,
                                   std::string& connection_id);

  const std::string& GetConnectionId(websocketpp::connection_hdl hdl);
  websocketpp::connection_hdl GetWsHandle(std::string& connection_id);

 private:
};

#endif