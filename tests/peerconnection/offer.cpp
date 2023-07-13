#include <iostream>

#include "rtc.h"

int main(int argc, char **argv) {
  CreatePeerConnection("ws://localhost:9002");

  getchar();
  return 0;
}
