#include <iostream>

#include "rtc.h"

int main(int argc, char **argv) {
  Params params;
  params.cfg_path = "../../../../config/config.ini";
  CreatePeerConnectionWithID(params, "000000");

  std::cout << "Finish CreatePeerConnectionWithID" << std::endl;

  std::string msg = "Hello world";

  int i = 100;
  while (i--) {
    getchar();
    std::cout << "Send data: [" << msg << "]" << std::endl;
    SendData(msg.data(), msg.size());
  }

  getchar();
  return 0;
}
