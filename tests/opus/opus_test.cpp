#include <fstream>
#include <iostream>
#include <vector>

// Opus���뺯��
#include <opus/opus.h>

#include <iostream>
#include <vector>

#define SAMPLE_RATE 48000
#define CHANNELS 2
#define FRAME_SIZE 960
#define APPLICATION OPUS_APPLICATION_AUDIO

// ���뺯��
int encode(const std::vector<opus_int16>& pcm,
           std::vector<unsigned char>& opus) {
  // ����������
  int error;
  OpusEncoder* encoder =
      opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, &error);
  if (error != OPUS_OK) {
    std::cerr << "Failed to create encoder: " << opus_strerror(error)
              << std::endl;
    return error;
  }

  // ���ñ���������
  opus_encoder_ctl(encoder, OPUS_SET_BITRATE(64000));

  // ������������С
  int maxOpusSize = FRAME_SIZE * CHANNELS * sizeof(opus_int16);
  opus.resize(maxOpusSize);

  // ����
  int encodedSize =
      opus_encode(encoder, pcm.data(), FRAME_SIZE, opus.data(), maxOpusSize);
  if (encodedSize < 0) {
    std::cerr << "Encoding error: " << opus_strerror(encodedSize) << std::endl;
    return encodedSize;
  }

  // ������Դ
  opus_encoder_destroy(encoder);

  // ������������Ĵ�С
  opus.resize(encodedSize);

  return 0;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " input.pcm output.opus" << std::endl;
    return -1;
  }

  // �������ļ�
  std::ifstream inputFile(argv[1], std::ios::binary);
  if (!inputFile) {
    std::cerr << "Failed to open input file." << std::endl;
    return -1;
  }

  // ��ȡPCM����
  std::vector<opus_int16> pcmData;
  opus_int16 sample;
  while (inputFile.read(reinterpret_cast<char*>(&sample), sizeof(opus_int16))) {
    pcmData.push_back(sample);
  }

  // ����ΪOpus��ʽ
  std::vector<unsigned char> opusData;
  int result = encode(pcmData, opusData);
  if (result != 0) {
    std::cerr << "Encoding failed with error code " << result << std::endl;
    return result;
  }

  // ������ļ�
  std::ofstream outputFile(argv[2], std::ios::binary);
  if (!outputFile) {
    std::cerr << "Failed to open output file." << std::endl;
    return -1;
  }

  // д��Opus����
  outputFile.write(reinterpret_cast<const char*>(opusData.data()),
                   opusData.size());

  // ���
  std::cout << "Encoding complete. size:" << pcmData.size() * 2 << std::endl;

  return 0;
}
