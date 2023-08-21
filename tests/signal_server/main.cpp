
#include <iostream>

#include "signal_server.h"

int main(int argc, char* argv[]) {
  SignalServer s;
  std::string port = "";
  if (argc > 1) {
    port = argv[1];
  } else {
    port = "9090";
  }
  std::cout << "Port: " << port << std::endl;
  s.run(std::stoi(port));
  return 0;
}