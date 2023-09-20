#ifndef _NVIDIA_VIDEO_DECODER_H_
#define _NVIDIA_VIDEO_DECODER_H_

#include <functional>

#include "NvDecoder.h"
#include "video_decoder.h"

class NvidiaVideoDecoder : public VideoDecoder {
 public:
  NvidiaVideoDecoder();
  ~NvidiaVideoDecoder();

 public:
  int Init();
  int Decode(const uint8_t* data, int size,
             std::function<void(VideoFrame)> on_receive_decoded_frame);

 private:
  NvDecoder* decoder = nullptr;
  bool get_first_keyframe_ = false;
  bool skip_frame_ = false;
  FILE* file_ = nullptr;
};

#endif