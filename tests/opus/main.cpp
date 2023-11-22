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

  // // ��ȡ������opus��һ����ڵ����̣߳�����ֻ��Ϊ�˷���
  // StreamInfo info;
  // while (opusEncoder.PopFrame(info)) {
  //   .....
  // }

  opusEncoder->Stop();

  return 0;
}