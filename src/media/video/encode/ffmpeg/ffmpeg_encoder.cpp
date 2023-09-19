#include "ffmpeg_encoder.h"

#include <chrono>

#include "log.h"

#define SAVE_ENCODER_STREAM 0

VideoEncoder::VideoEncoder() {
  if (SAVE_ENCODER_STREAM) {
    file_ = fopen("encode_stream.h264", "w+b");
    if (!file_) {
      LOG_WARN("Fail to open stream.h264");
    }
  }
}
VideoEncoder::~VideoEncoder() {
  if (SAVE_ENCODER_STREAM && file_) {
    fflush(file_);
    fclose(file_);
    file_ = nullptr;
  }

  if (nv12_data_) {
    free(nv12_data_);
    nv12_data_ = nullptr;
  }
}

int VideoEncoder::Init() { return 0; }

int VideoEncoder::Encode(
    const uint8_t *pData, int nSize,
    std::function<int(char *encoded_packets, size_t size)> on_encoded_image) {
  return -1;
}

int VideoEncoder::OnEncodedImage(char *encoded_packets, size_t size) {
  LOG_INFO("output encoded image");
  fwrite(encoded_packets, 1, size, file_);
  return 0;
}

void VideoEncoder::ForceIdr() {}
