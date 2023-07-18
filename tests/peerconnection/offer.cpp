#include <iostream>

#include "rtc.h"

int main(int argc, char **argv) {
  Params params;
  params.cfg_path = "/home/zjlab/data/djk/projectx/config/config.ini";
  CreatePeerConnection(params);

  getchar();
  return 0;
}
