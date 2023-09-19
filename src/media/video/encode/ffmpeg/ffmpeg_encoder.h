#ifndef _FFMPEG_ENCODER_H_
#define _FFMPEG_ENCODER_H_

#ifdef _WIN32
extern "C" {
#include "libavcodec/avcodec.h"
};
#else
#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
};
#endif
#endif
#include <functional>

class VideoEncoder {
 public:
  VideoEncoder();
  ~VideoEncoder();

  int Init();
  int Encode(
      const uint8_t* pData, int nSize,
      std::function<int(char* encoded_packets, size_t size)> on_encoded_image);

  virtual int OnEncodedImage(char* encoded_packets, size_t size);

  void ForceIdr();

 private:
  int frame_width = 1280;
  int frame_height = 720;
  int keyFrameInterval_ = 3000;
  int maxBitrate_ = 2000;
  int max_payload_size_ = 3000;

  std::vector<std::vector<uint8_t>> encoded_packets_;
  unsigned char* encoded_image_ = nullptr;
  FILE* file_ = nullptr;
  unsigned char* nv12_data_ = nullptr;
  unsigned int seq_ = 0;
};

#endif