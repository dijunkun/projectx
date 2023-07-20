#include <iostream>

#include "x.h"

int main(int argc, char** argv) {
  Params params;
  params.cfg_path = "../../../../config/config.ini";

  PeerPtr* peer = CreatePeer(&params);
  CreateConnection(peer);

  getchar();
  return 0;
}
