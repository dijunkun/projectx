#include <fstream>
#include <iostream>
#include <vector>

#include "OpusEncoderImpl.h"
#include "opus/opus.h"

int main() {
  OpusEncoderImpl* opusEncoder = new OpusEncoderImpl(48000, 2);

  std::ifstream inputFile("ls.pcm", std::ios::binary);
  if (!inputFile) {
    std::cerr << "Failed to open input file." << std::endl;
    return -1;
  }

  char sample[960];
  while (inputFile.read(sample, 960)) {
    opusEncoder->Feed((unsigned char*)sample, 960);
  }

  // // 读取编码后的opus，一般放在单独线程，这里只是为了方便
  // StreamInfo info;
  // while (opusEncoder.PopFrame(info)) {
  //   .....
  // }

  opusEncoder->Stop();

  return 0;
}