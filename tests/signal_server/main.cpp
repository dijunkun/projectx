
#include <iostream>

#include "signal_server.h"

int main(int argc, char* argv[]) {
  SignalServer s;
  std::string port = argv[1];
  std::cout << "Port: " << port << std::endl;
  s.run(std::stoi(port));
  return 0;
}