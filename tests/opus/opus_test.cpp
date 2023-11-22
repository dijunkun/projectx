#include <fstream>
#include <iostream>
#include <vector>

// Opus编码函数
#include <opus/opus.h>

#include <iostream>
#include <vector>

#define SAMPLE_RATE 48000
#define CHANNELS 2
#define FRAME_SIZE 960
#define APPLICATION OPUS_APPLICATION_AUDIO

// 编码函数
int encode(const std::vector<opus_int16>& pcm,
           std::vector<unsigned char>& opus) {
  // 创建编码器
  int error;
  OpusEncoder* encoder =
      opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, &error);
  if (error != OPUS_OK) {
    std::cerr << "Failed to create encoder: " << opus_strerror(error)
              << std::endl;
    return error;
  }

  // 设置编码器参数
  opus_encoder_ctl(encoder, OPUS_SET_BITRATE(64000));

  // 计算最大输出大小
  int maxOpusSize = FRAME_SIZE * CHANNELS * sizeof(opus_int16);
  opus.resize(maxOpusSize);

  // 编码
  int encodedSize =
      opus_encode(encoder, pcm.data(), FRAME_SIZE, opus.data(), maxOpusSize);
  if (encodedSize < 0) {
    std::cerr << "Encoding error: " << opus_strerror(encodedSize) << std::endl;
    return encodedSize;
  }

  // 清理资源
  opus_encoder_destroy(encoder);

  // 调整输出向量的大小
  opus.resize(encodedSize);

  return 0;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " input.pcm output.opus" << std::endl;
    return -1;
  }

  // 打开输入文件
  std::ifstream inputFile(argv[1], std::ios::binary);
  if (!inputFile) {
    std::cerr << "Failed to open input file." << std::endl;
    return -1;
  }

  // 读取PCM数据
  std::vector<opus_int16> pcmData;
  opus_int16 sample;
  while (inputFile.read(reinterpret_cast<char*>(&sample), sizeof(opus_int16))) {
    pcmData.push_back(sample);
  }

  // 编码为Opus格式
  std::vector<unsigned char> opusData;
  int result = encode(pcmData, opusData);
  if (result != 0) {
    std::cerr << "Encoding failed with error code " << result << std::endl;
    return result;
  }

  // 打开输出文件
  std::ofstream outputFile(argv[2], std::ios::binary);
  if (!outputFile) {
    std::cerr << "Failed to open output file." << std::endl;
    return -1;
  }

  // 写入Opus数据
  outputFile.write(reinterpret_cast<const char*>(opusData.data()),
                   opusData.size());

  // 完成
  std::cout << "Encoding complete. size:" << pcmData.size() * 2 << std::endl;

  return 0;
}
