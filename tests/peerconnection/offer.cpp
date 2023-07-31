#include <iostream>

#include "x.h"

int main(int argc, char** argv) {
  Params params;
  params.cfg_path = "../../../../config/config.ini";

  PeerPtr* peer = CreatePeer(&params);
  CreateConnection(peer);

  std::string msg = "Answer peer";

  int i = 100;
  while (i--) {
    getchar();
    std::cout << "Send data: [" << msg << "]" << std::endl;
    SendData(peer, msg.data(), msg.size());
  }

  getchar();
  return 0;
}
