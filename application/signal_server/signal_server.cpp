#include "signal_server.h"

#include "common.h"
#include "log.h"

const std::string GenerateTransmissionId() {
  static const char alphanum[] = "0123456789";
  std::string random_id;
  random_id.reserve(6);

  for (int i = 0; i < 6; ++i) {
    random_id += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return "000000";
}

SignalServer::SignalServer() {
  // Set logging settings
  server_.set_error_channels(websocketpp::log::elevel::all);
  server_.set_access_channels(websocketpp::log::alevel::none);

  // Initialize Asio
  server_.init_asio();

  server_.set_open_handler(
      std::bind(&SignalServer::on_open, this, std::placeholders::_1));

  server_.set_close_handler(
      std::bind(&SignalServer::on_close, this, std::placeholders::_1));

  server_.set_message_handler(std::bind(&SignalServer::on_message, this,
                                        std::placeholders::_1,
                                        std::placeholders::_2));

  server_.set_ping_handler(bind(&SignalServer::on_ping, this,
                                std::placeholders::_1, std::placeholders::_2));

  server_.set_pong_handler(bind(&SignalServer::on_pong, this,
                                std::placeholders::_1, std::placeholders::_2));
}

SignalServer::~SignalServer() {}

bool SignalServer::on_open(websocketpp::connection_hdl hdl) {
  ws_connections_[hdl] = ws_connection_id_++;
  LOG_INFO("New websocket connection [{}] established", ws_connection_id_);

  json message = {{"type", "ws_connection_id"},
                  {"ws_connection_id", ws_connection_id_}};
  server_.send(hdl, message.dump(), websocketpp::frame::opcode::text);

  return true;
}

bool SignalServer::on_close(websocketpp::connection_hdl hdl) {
  LOG_INFO("Websocket onnection [{}] closed", ws_connection_id_);
  std::string user_id = transmission_manager_.GetUserId(hdl);
  std::string transmission_id =
      transmission_manager_.ReleaseUserIdFromTransmission(hdl);

  if (!transmission_id.empty()) {
    json message = {{"type", "user_leave_transmission"},
                    {"transmission_id", transmission_id},
                    {"user_id", user_id}};

    std::vector<std::string> user_id_list =
        transmission_manager_.GetAllUserIdOfTransmission(transmission_id);

    if (user_id_list.empty()) {
      transmission_list_.erase(transmission_id);
      LOG_INFO("Release transmission [{}] due to no user in this transmission",
               transmission_id);
    }

    if (std::string::npos != user_id.find("S-")) {
      transmission_list_.erase(transmission_id);
      transmission_manager_.ReleaseAllUserIdFromTransmission(transmission_id);
      LOG_INFO("Release transmission [{}] due to server leaves",
               transmission_id);
    }

    for (const auto& user_id : user_id_list) {
      send_msg(transmission_manager_.GetWsHandle(user_id), message);
    }
  }
  ws_connections_.erase(hdl);

  return true;
}

bool SignalServer::on_ping(websocketpp::connection_hdl hdl, std::string s) {
  /* Do something */
  LOG_INFO("Receive ping");
  return true;
}

bool SignalServer::on_pong(websocketpp::connection_hdl hdl, std::string s) {
  /* Do something */
  LOG_INFO("pong");
  return true;
}

void SignalServer::run(uint16_t port) {
  LOG_INFO("Signal server runs on port [{}]", port);

  server_.set_reuse_addr(true);
  server_.listen(port);

  // Queues a connection accept operation
  server_.start_accept();

  // Start the Asio io_service run loop
  server_.run();
}

void SignalServer::send_msg(websocketpp::connection_hdl hdl, json message) {
  if (!hdl.expired()) {
    server_.send(hdl, message.dump(), websocketpp::frame::opcode::text);
  }
}

void SignalServer::on_message(websocketpp::connection_hdl hdl,
                              server::message_ptr msg) {
  std::string payload = msg->get_payload();

  auto j = json::parse(payload);
  std::string type = j["type"].get<std::string>();

  switch (HASH_STRING_PIECE(type.c_str())) {
    case "create_transmission"_H: {
      std::string transmission_id = j["transmission_id"].get<std::string>();
      std::string password = j["password"].get<std::string>();
      std::string user_id = j["user_id"].get<std::string>();
      LOG_INFO("Receive user id [{}] create transmission request with id [{}]",
               user_id, transmission_id);
      if (transmission_list_.find(transmission_id) ==
          transmission_list_.end()) {
        if (transmission_id.empty()) {
          transmission_id = GenerateTransmissionId();
          while (transmission_list_.find(transmission_id) !=
                 transmission_list_.end()) {
            transmission_id = GenerateTransmissionId();
          }
          LOG_INFO(
              "Transmission id is empty, generate a new one for this request "
              "[{}]",
              transmission_id);
        }
        transmission_list_.insert(transmission_id);

        transmission_manager_.BindUserIdToTransmission(user_id,
                                                       transmission_id);
        transmission_manager_.BindUserIdToWsHandle(user_id, hdl);
        transmission_manager_.BindPasswordToTransmission(password,
                                                         transmission_id);

        LOG_INFO("Create transmission id [{}]", transmission_id);
        json message = {{"type", "transmission_id"},
                        {"transmission_id", transmission_id},
                        {"status", "success"}};
        send_msg(hdl, message);
      } else {
        LOG_INFO("Transmission id [{}] already exist", transmission_id);
        json message = {{"type", "transmission_id"},
                        {"transmission_id", transmission_id},
                        {"status", "fail"},
                        {"reason", "Transmission id exist"}};
        send_msg(hdl, message);
      }

      break;
    }
    case "leave_transmission"_H: {
      std::string transmission_id = j["transmission_id"].get<std::string>();
      std::string user_id = j["user_id"].get<std::string>();
      LOG_INFO("[{}] leaves transmission [{}]", user_id.c_str(),
               transmission_id.c_str());

      json message = {{"type", "user_leave_transmission"},
                      {"transmission_id", transmission_id},
                      {"user_id", user_id}};

      std::vector<std::string> user_id_list =
          transmission_manager_.GetAllUserIdOfTransmission(transmission_id);

      for (const auto& user_id : user_id_list) {
        send_msg(transmission_manager_.GetWsHandle(user_id), message);
      }

      transmission_manager_.ReleaseUserIdFromTransmission(hdl);
      if (std::string::npos != user_id.find("S-")) {
        transmission_list_.erase(transmission_id);
        transmission_manager_.ReleaseAllUserIdFromTransmission(transmission_id);
        LOG_INFO("Release transmission [{}] due to server leaves",
                 transmission_id);
      }

      break;
    }
    case "query_user_id_list"_H: {
      std::string transmission_id = j["transmission_id"].get<std::string>();
      std::string password = j["password"].get<std::string>();

      if (transmission_manager_.CheckPassword(password, transmission_id)) {
        std::vector<std::string> user_id_list =
            transmission_manager_.GetAllUserIdOfTransmission(transmission_id);

        json message = {{"type", "user_id_list"},
                        {"transmission_id", transmission_id},
                        {"user_id_list", user_id_list},
                        {"status", "success"}};

        send_msg(hdl, message);
      } else {
        std::vector<std::string> user_id_list;
        json message = {{"type", "user_id_list"},
                        {"transmission_id", transmission_id},
                        {"user_id_list", user_id_list},
                        {"status", "failed"},
                        {"reason", "Incorrect password"}};
        // LOG_INFO(
        //     "Incorrect password [{}] for transmission [{}] with password is "
        //     "[{}]",
        //     password, transmission_id,
        //     transmission_manager_.GetPassword(transmission_id));

        send_msg(hdl, message);
      }

      // LOG_INFO("Send member_list: [{}]", message.dump());

      break;
    }
    case "offer"_H: {
      std::string transmission_id = j["transmission_id"].get<std::string>();
      std::string sdp = j["sdp"].get<std::string>();
      std::string user_id = j["user_id"].get<std::string>();
      std::string remote_user_id = j["remote_user_id"].get<std::string>();

      transmission_manager_.BindUserIdToTransmission(user_id, transmission_id);
      transmission_manager_.BindUserIdToWsHandle(user_id, hdl);

      websocketpp::connection_hdl destination_hdl =
          transmission_manager_.GetWsHandle(remote_user_id);

      json message = {{"type", "offer"},
                      {"sdp", sdp},
                      {"remote_user_id", user_id},
                      {"transmission_id", transmission_id}};

      LOG_INFO("[{}] send offer to [{}]", user_id, remote_user_id);
      send_msg(destination_hdl, message);

      break;
    }
    case "answer"_H: {
      std::string transmission_id = j["transmission_id"].get<std::string>();
      std::string sdp = j["sdp"].get<std::string>();
      std::string user_id = j["user_id"].get<std::string>();
      std::string remote_user_id = j["remote_user_id"].get<std::string>();

      websocketpp::connection_hdl destination_hdl =
          transmission_manager_.GetWsHandle(remote_user_id);

      // LOG_INFO("send answer sdp [{}]", sdp);
      LOG_INFO("[{}] send answer to [{}]", user_id, remote_user_id);
      json message = {{"type", "answer"},
                      {"sdp", sdp},
                      {"remote_user_id", user_id},
                      {"transmission_id", transmission_id}};
      send_msg(destination_hdl, message);
      break;
    }
    case "offer_candidate"_H: {
      std::string transmission_id = j["transmission_id"].get<std::string>();
      std::string candidate = j["sdp"].get<std::string>();
      LOG_INFO("send candidate [{}]", candidate.c_str());
      json message = {{"type", "candidate"}, {"sdp", candidate}};
      // send_msg(answer_hdl_map_[transmission_id], message);
      break;
    }
    case "answer_candidate"_H: {
      std::string transmission_id = j["transmission_id"].get<std::string>();
      std::string candidate = j["sdp"].get<std::string>();
      LOG_INFO("send candidate [{}]", candidate.c_str());
      json message = {{"type", "candidate"}, {"sdp", candidate}};
      // send_msg(offer_hdl_map_[transmission_id], message);
      break;
    }
    default:
      break;
  }

  // std::string sdp = j["sdp"];

  // LOG_INFO("Message type: {}", type);
  // LOG_INFO("Message body: {}", sdp);

  // server_.send(hdl, msg->get_payload(), msg->get_opcode());
}