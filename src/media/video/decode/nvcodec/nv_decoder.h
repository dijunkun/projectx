#ifndef _NV_DECODER_H_
#define _NV_DECODER_H_

#include <functional>

#include "NvDecoder.h"
#include "frame.h"

class VideoDecoder {
 public:
  VideoDecoder();
  ~VideoDecoder();

 public:
  int Init();
  int Decode(const uint8_t* pData, int nSize,
             std::function<void(VideoFrame)> on_receive_decoded_frame);
  int GetFrame(uint8_t* yuv_data, uint32_t& width, uint32_t& height,
               uint32_t& size);

 private:
  NvDecoder* decoder = nullptr;
  bool get_first_keyframe_ = false;
  bool skip_frame_ = false;
  FILE* file_ = nullptr;
};

#endif