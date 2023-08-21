#include <iostream>

#include "x.h"

int main(int argc, char** argv) {
  Params params;
  params.cfg_path = "../../../../config/config.ini";

  std::string transmission_id = "000000";
  std::string user_id = argv[1];
  // std::cout << "Please input which transmisson want to join: ";
  // std::cin >> transmission_id;

  PeerPtr* peer = CreatePeer(&params);
  JoinConnection(peer, transmission_id.c_str(), user_id.c_str());

  std::string msg = "Offer peer";

  int i = 100;
  while (i--) {
    getchar();
    std::cout << "Send data: [" << msg << "]" << std::endl;
    SendData(peer, msg.data(), msg.size());
  }

  getchar();
  return 0;
}
