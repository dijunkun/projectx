#include <iostream>

#include "rtc.h"

int main(int argc, char **argv) {
  CreatePeerConnectionWithID("ws://localhost:9002", "000000");

  getchar();
  return 0;
}
