#include "signal_server.h"

#include "log.h"

static const std::map<std::string, unsigned int> siganl_types{
    {"create_transport", 1}, {"offer", 2},           {"query_remote_sdp", 3},
    {"answer", 4},           {"offer_candidate", 5}, {"answer_candidate", 6}};

std::string gen_random_6() {
  static const char alphanum[] = "0123456789";
  std::string tmp_s;
  tmp_s.reserve(6);

  for (int i = 0; i < 6; ++i) {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  //   return tmp_s;
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
  connections_[hdl] = connection_id_;
  LOG_INFO("New connection [{}] established", connection_id_++);

  json message = {{"type", "connection_id"}, {"connection_id", connection_id_}};
  server_.send(hdl, message.dump(), websocketpp::frame::opcode::text);

  return true;
}

bool SignalServer::on_close(websocketpp::connection_hdl hdl) {
  LOG_INFO("Connection [{}] closed", connection_id_++);
  connections_.erase(hdl);
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
  auto itr = siganl_types.find(type);
  if (itr != siganl_types.end()) {
    LOG_INFO("msg type: {}", itr->first);
    switch (itr->second) {
      case 1: {
        transport_id_ = gen_random_6();
        LOG_INFO("Generate transport_id [{}]", transport_id_);
        json message = {{"type", "transport_id"},
                        {"transport_id", transport_id_}};
        send_msg(hdl, message);
        break;
      }
      case 2: {
        std::string transport_id = j["transport_id"];
        std::string sdp = j["sdp"];
        LOG_INFO("Save transport_id[{}] with offer sdp[{}]", transport_id, sdp);
        offer_sdp_map_[transport_id] = sdp;
        offer_hdl_map_[transport_id] = hdl;
        break;
      }
      case 3: {
        std::string transport_id = j["transport_id"];
        std::string sdp = offer_sdp_map_[transport_id_];
        LOG_INFO("send offer sdp [{}]", sdp.c_str());
        json message = {{"type", "remote_sdp"}, {"sdp", sdp}};
        send_msg(hdl, message);
        break;
      }
      case 4: {
        std::string transport_id = j["transport_id"];
        std::string sdp = j["sdp"];
        LOG_INFO("Save transport_id[{}] with answer sdp[{}]", transport_id,
                 sdp);
        answer_sdp_map_[transport_id] = sdp;
        answer_hdl_map_[transport_id] = hdl;
        LOG_INFO("send answer sdp [{}]", sdp.c_str());
        json message = {{"type", "remote_sdp"}, {"sdp", sdp}};
        send_msg(offer_hdl_map_[transport_id], message);
        break;
      }
      case 5: {
        std::string transport_id = j["transport_id"];
        std::string candidate = j["sdp"];
        LOG_INFO("send candidate [{}]", candidate.c_str());
        json message = {{"type", "candidate"}, {"sdp", candidate}};
        send_msg(answer_hdl_map_[transport_id], message);
        break;
      }
      case 6: {
        std::string transport_id = j["transport_id"];
        std::string candidate = j["sdp"];
        LOG_INFO("send candidate [{}]", candidate.c_str());
        json message = {{"type", "candidate"}, {"sdp", candidate}};
        send_msg(offer_hdl_map_[transport_id], message);
        break;
      }
      default:
        break;
    }
  }

  // std::string sdp = j["sdp"];

  // LOG_INFO("Message type: {}", type);
  // LOG_INFO("Message body: {}", sdp);

  // server_.send(hdl, msg->get_payload(), msg->get_opcode());
}