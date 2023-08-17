#include "signal_server.h"

#include "log.h"

constexpr size_t HASH_STRING_PIECE(const char* string_piece) {
  std::size_t result = 0;
  while (*string_piece) {
    result = (result * 131) + *string_piece++;
  }
  return result;
}

constexpr size_t operator"" _H(const char* string_piece, size_t) {
  return HASH_STRING_PIECE(string_piece);
}

const std::string gen_random_6() {
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
  LOG_INFO("Websocket onnection [{}] closed", ws_connection_id_++);
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

void SignalServer::run() {
  // Listen on port 9002
  server_.listen(9002);

  // Queues a connection accept operation
  server_.start_accept();

  // Start the Asio io_service run loop
  server_.run();
}

void SignalServer::send_msg(websocketpp::connection_hdl hdl, json message) {
  server_.send(hdl, message.dump(), websocketpp::frame::opcode::text);
}

void SignalServer::on_message(websocketpp::connection_hdl hdl,
                              server::message_ptr msg) {
  std::string payload = msg->get_payload();

  auto j = json::parse(payload);
  std::string type = j["type"];

  switch (HASH_STRING_PIECE(type.c_str())) {
    case "create_transmission"_H: {
      transmission_id_ = j["transmission_id"];
      LOG_INFO("Receive create transmission request with id [{}]",
               transmission_id_);
      if (transmission_list_.find(transmission_id_) ==
          transmission_list_.end()) {
        if (transmission_id_.empty()) {
          transmission_id_ = gen_random_6();
          while (transmission_list_.find(transmission_id_) !=
                 transmission_list_.end()) {
            transmission_id_ = gen_random_6();
          }
          LOG_INFO(
              "Transmission id is empty, generate a new one for this request "
              "[{}]",
              transmission_id_);
        }
        transmission_list_.insert(transmission_id_);
        LOG_INFO("Create transmission id [{}]", transmission_id_);
        json message = {{"type", "transmission_id"},
                        {"transmission_id", transmission_id_},
                        {"status", "success"}};
        send_msg(hdl, message);
      } else {
        LOG_INFO("Transmission id [{}] already exist", transmission_id_);
        json message = {{"type", "transmission_id"},
                        {"transmission_id", transmission_id_},
                        {"status", "fail"},
                        {"reason", "Transmission id exist"}};
        send_msg(hdl, message);
      }

      break;
    }
    case "offer"_H: {
      std::string transmission_id = j["transmission_id"];
      std::string sdp = j["sdp"];
      LOG_INFO("Save transmission_id[{}] with offer sdp[{}]", transmission_id,
               sdp);
      // ws_handle_manager_.BindHandleToConnection(hdl, );
      offer_sdp_map_[transmission_id] = sdp;
      offer_hdl_map_[transmission_id] = hdl;
      break;
    }
    case "query_remote_sdp"_H: {
      std::string transmission_id = j["transmission_id"];
      std::string sdp = offer_sdp_map_[transmission_id];
      LOG_INFO("send offer sdp [{}]", sdp.c_str());
      json message = {{"type", "remote_sdp"}, {"sdp", sdp}};
      send_msg(hdl, message);
      break;
    }
    case "answer"_H: {
      std::string transmission_id = j["transmission_id"];
      std::string sdp = j["sdp"];
      LOG_INFO("Save transmission_id[{}] with answer sdp[{}]", transmission_id,
               sdp);
      answer_sdp_map_[transmission_id] = sdp;
      answer_hdl_map_[transmission_id] = hdl;
      LOG_INFO("send answer sdp [{}]", sdp.c_str());
      json message = {{"type", "remote_sdp"}, {"sdp", sdp}};
      send_msg(offer_hdl_map_[transmission_id], message);
      break;
    }
    case "offer_candidate"_H: {
      std::string transmission_id = j["transmission_id"];
      std::string candidate = j["sdp"];
      LOG_INFO("send candidate [{}]", candidate.c_str());
      json message = {{"type", "candidate"}, {"sdp", candidate}};
      send_msg(answer_hdl_map_[transmission_id], message);
      break;
    }
    case "answer_candidate"_H: {
      std::string transmission_id = j["transmission_id"];
      std::string candidate = j["sdp"];
      LOG_INFO("send candidate [{}]", candidate.c_str());
      json message = {{"type", "candidate"}, {"sdp", candidate}};
      send_msg(offer_hdl_map_[transmission_id], message);
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